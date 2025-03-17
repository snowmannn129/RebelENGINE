#include "graphics/ViewTransitionManager.h"
#include <algorithm>
#include <cmath>

namespace RebelCAD {
namespace Graphics {

ViewTransitionManager::ViewTransitionManager(
    std::shared_ptr<SceneGraph> sceneGraph,
    std::shared_ptr<CameraCollision> collisionSystem
) : m_sceneGraph(sceneGraph),
    m_collisionSystem(collisionSystem),
    m_nextTransitionId(1),
    m_pathSmoothingFactor(0.5f),
    m_collisionPaddingDefault(0.1f),
    m_optimizationPassesDefault(3),
    m_speedAdjustmentFactor(0.5f),
    m_waypointBlendingDefault(0.2f)
{
}

std::vector<BezierControlPoint> ViewTransitionManager::generateBezierPath(
    const glm::vec3& start,
    const glm::vec3& end,
    const PathPlanningParams& params
) {
    std::vector<BezierControlPoint> controlPoints;
    
    // Start point with increased influence for better initial direction control
    controlPoints.push_back({start, params.collisionPadding * 1.5f});
    
    // Generate intermediate control points with adaptive spacing
    glm::vec3 direction = end - start;
    float distance = glm::length(direction);
    direction = glm::normalize(direction);
    
    if (!params.waypoints.empty()) {
        // Process waypoints with dynamic influence radii
        float totalDistance = 0.0f;
        glm::vec3 lastPos = start;
        
        for (const auto& waypoint : params.waypoints) {
            float segmentDist = glm::distance(lastPos, waypoint.position);
            totalDistance += segmentDist;
            
            // Adjust influence radius based on distance and surrounding geometry
            float dynamicInfluence = waypoint.blendRadius;
            
            // Check for nearby obstacles and adjust influence
            Viewport tempViewport;
            tempViewport.setView(lastPos, waypoint.position, glm::vec3(0, 1, 0));
            if (m_collisionSystem->predictCollision(tempViewport, *m_sceneGraph, waypoint.position)) {
                dynamicInfluence *= 1.5f; // Increase influence near obstacles
            }
            
            controlPoints.push_back({waypoint.position, dynamicInfluence});
            lastPos = waypoint.position;
        }
        
        // Add intermediate points for long segments
        if (params.adaptiveSpeed) {
            for (size_t i = 0; i < controlPoints.size() - 1; ++i) {
                float segmentDist = glm::distance(controlPoints[i].position, controlPoints[i + 1].position);
                if (segmentDist > distance * 0.25f) { // Long segment
                    glm::vec3 midPoint = (controlPoints[i].position + controlPoints[i + 1].position) * 0.5f;
                    float midInfluence = (controlPoints[i].influence + controlPoints[i + 1].influence) * 0.5f;
                    controlPoints.insert(controlPoints.begin() + i + 1, {midPoint, midInfluence});
                }
            }
        }
    } else {
        // Generate adaptive control points based on scene complexity
        const size_t basePoints = 4; // Minimum number of control points
        size_t numPoints = basePoints;
        
        // Sample points along direct path to check complexity
        std::vector<float> complexityScores;
        for (size_t i = 1; i < basePoints; ++i) {
            float t = static_cast<float>(i) / basePoints;
            glm::vec3 samplePoint = start + direction * (distance * t);
            
            // Check for obstacles near sample point
            Viewport tempViewport;
            tempViewport.setView(start, samplePoint, glm::vec3(0, 1, 0));
            if (m_collisionSystem->predictCollision(tempViewport, *m_sceneGraph, samplePoint)) {
                numPoints++; // Add more control points in complex areas
            }
        }
        
        // Generate control points with adaptive spacing
        for (size_t i = 1; i < numPoints - 1; ++i) {
            float t = static_cast<float>(i) / (numPoints - 1);
            glm::vec3 point = start + direction * (distance * t);
            
            // Adjust influence based on local complexity
            float influence = params.collisionPadding;
            Viewport tempViewport;
            tempViewport.setView(start, point, glm::vec3(0, 1, 0));
            if (m_collisionSystem->predictCollision(tempViewport, *m_sceneGraph, point)) {
                influence *= 1.5f; // Increase influence in complex areas
            }
            
            controlPoints.push_back({point, influence});
        }
    }
    
    // End point with increased influence for better final approach
    controlPoints.push_back({end, params.collisionPadding * 1.5f});
    
    return optimizePath(controlPoints, params);
}

std::vector<BezierControlPoint> ViewTransitionManager::optimizePath(
    const std::vector<BezierControlPoint>& initialPoints,
    const PathPlanningParams& params
) {
    std::vector<BezierControlPoint> optimized = initialPoints;
    
    // Energy terms for optimization
    const float kSmooth = 0.4f;    // Smoothness weight
    const float kObstacle = 0.6f;  // Obstacle avoidance weight
    const float kLength = 0.3f;    // Path length weight
    const float kCurvature = 0.4f; // Curvature weight
    
    // Perform multiple optimization passes
    for (size_t pass = 0; pass < params.optimizationPasses; ++pass) {
        // Forward pass
        for (size_t i = 1; i < optimized.size() - 1; ++i) {
            glm::vec3& current = optimized[i].position;
            float& influence = optimized[i].influence;
            
            // Create temporary viewport for collision testing
            Viewport tempViewport;
            tempViewport.setView(current, optimized[i + 1].position, glm::vec3(0, 1, 0));
            
            // Obstacle avoidance
            if (m_collisionSystem->predictCollision(tempViewport, *m_sceneGraph, current)) {
                auto collisionResult = m_collisionSystem->testCollision(tempViewport, *m_sceneGraph);
                if (collisionResult.hasCollision) {
                    // Adaptive collision response
                    float responseStrength = kObstacle * (1.0f + collisionResult.penetrationDepth);
                    current += collisionResult.contactNormal * 
                        (params.collisionPadding + collisionResult.penetrationDepth) * responseStrength;
                    
                    // Increase influence radius near obstacles
                    influence *= 1.2f;
                }
                
                // Find safe position with increased search radius
                current = m_collisionSystem->resolveCollision(tempViewport, *m_sceneGraph, current);
            }
            
            // Path smoothing with curvature minimization
            if (i > 0 && i < optimized.size() - 1) {
                glm::vec3 prev = optimized[i - 1].position;
                glm::vec3 next = optimized[i + 1].position;
                
                // Compute local curvature
                glm::vec3 v1 = glm::normalize(current - prev);
                glm::vec3 v2 = glm::normalize(next - current);
                float curvature = 1.0f - glm::dot(v1, v2);
                
                // Smooth position based on curvature
                glm::vec3 ideal = (prev + next) * 0.5f;
                float smoothWeight = kSmooth * (1.0f - curvature * kCurvature);
                current = glm::mix(current, ideal, smoothWeight * params.pathSmoothing);
                
                // Length preservation
                float targetLength = glm::distance(prev, next) * 0.5f;
                glm::vec3 toPrev = prev - current;
                glm::vec3 toNext = next - current;
                float actualLength = (glm::length(toPrev) + glm::length(toNext)) * 0.5f;
                
                if (actualLength > targetLength) {
                    current += (toPrev + toNext) * kLength * (targetLength / actualLength - 1.0f);
                }
            }
        }
        
        // Backward pass for additional smoothing
        for (size_t i = optimized.size() - 2; i > 0; --i) {
            glm::vec3& current = optimized[i].position;
            glm::vec3 prev = optimized[i - 1].position;
            glm::vec3 next = optimized[i + 1].position;
            
            // Apply reverse smoothing with reduced weight
            glm::vec3 ideal = (prev + next) * 0.5f;
            current = glm::mix(current, ideal, params.pathSmoothing * 0.5f);
        }
    }
    
    return optimized;
}

glm::vec3 ViewTransitionManager::evaluateBezierCurve(
    const std::vector<BezierControlPoint>& controlPoints,
    float t
) {
    if (controlPoints.size() < 2) return controlPoints[0].position;
    
    std::vector<glm::vec3> points;
    for (const auto& cp : controlPoints) {
        points.push_back(cp.position);
    }
    
    // De Casteljau's algorithm
    while (points.size() > 1) {
        std::vector<glm::vec3> newPoints;
        for (size_t i = 0; i < points.size() - 1; ++i) {
            newPoints.push_back(glm::mix(points[i], points[i + 1], t));
        }
        points = std::move(newPoints);
    }
    
    return points[0];
}

float ViewTransitionManager::updateTransitionSpeed(
    TransitionState& state,
    float deltaTime
) {
    if (!state.params.accelParams) {
        return state.currentSpeed;
    }
    
    const auto& accel = *state.params.accelParams;
    float targetSpeed = accel.maxSpeed;
    
    // Advanced speed adjustment based on multiple factors
    if (accel.adaptToPath && !state.optimizedPath.empty()) {
        size_t currentIndex = static_cast<size_t>(state.progress * (state.optimizedPath.size() - 1));
        if (currentIndex < state.optimizedPath.size() - 1) {
            // Calculate local path properties
            glm::vec3 currentPos = state.optimizedPath[currentIndex];
            glm::vec3 nextPos = state.optimizedPath[currentIndex + 1];
            glm::vec3 currentDir = glm::normalize(nextPos - currentPos);
            
            // Curvature-based speed adjustment
            float curvature = 0.0f;
            if (currentIndex > 0) {
                glm::vec3 prevDir = glm::normalize(
                    currentPos - state.optimizedPath[currentIndex - 1]);
                curvature = 1.0f - glm::dot(currentDir, prevDir);
            }
            
            // Look ahead for upcoming features
            float lookaheadCurvature = 0.0f;
            size_t lookaheadPoints = 3;
            for (size_t i = 1; i <= lookaheadPoints && currentIndex + i < state.optimizedPath.size() - 1; ++i) {
                glm::vec3 p1 = state.optimizedPath[currentIndex + i];
                glm::vec3 p2 = state.optimizedPath[currentIndex + i + 1];
                glm::vec3 dir = glm::normalize(p2 - p1);
                lookaheadCurvature += 1.0f - glm::dot(currentDir, dir);
            }
            lookaheadCurvature /= lookaheadPoints;
            
            // Check for nearby obstacles
            float obstacleProximity = 0.0f;
            Viewport tempViewport;
            tempViewport.setView(currentPos, nextPos, glm::vec3(0, 1, 0));
            if (m_collisionSystem->predictCollision(tempViewport, *m_sceneGraph, nextPos)) {
                auto collisionResult = m_collisionSystem->testCollision(tempViewport, *m_sceneGraph);
                if (collisionResult.hasCollision) {
                    obstacleProximity = 1.0f - std::min(1.0f, 
                        collisionResult.penetrationDepth / (accel.maxSpeed * deltaTime));
                }
            }
            
            // Combine factors for final speed adjustment
            float curvatureWeight = 0.4f;
            float lookaheadWeight = 0.3f;
            float obstacleWeight = 0.3f;
            
            float speedFactor = 1.0f - (
                curvature * curvatureWeight +
                lookaheadCurvature * lookaheadWeight +
                obstacleProximity * obstacleWeight
            );
            
            targetSpeed *= std::max(0.2f, speedFactor); // Maintain minimum speed
        }
    }
    
    // Smooth acceleration/deceleration with adaptive rates
    float currentDistance = glm::distance(
        state.start.position,
        state.optimizedPath[static_cast<size_t>(state.progress * (state.optimizedPath.size() - 1))]
    );
    float totalDistance = state.pathDistance;
    float distanceRatio = currentDistance / totalDistance;
    
    // Adjust acceleration/deceleration based on progress
    float adaptiveAccel = accel.acceleration;
    float adaptiveDecel = accel.deceleration;
    
    if (distanceRatio < 0.3f) {
        // Startup phase - gradual acceleration
        adaptiveAccel *= (0.5f + distanceRatio * 1.67f);
    } else if (distanceRatio > 0.7f) {
        // Approach phase - increased deceleration
        adaptiveDecel *= (1.0f + (distanceRatio - 0.7f) * 2.0f);
    }
    
    // Apply adaptive acceleration/deceleration
    if (state.currentSpeed < targetSpeed) {
        state.currentSpeed += adaptiveAccel * deltaTime;
        state.currentSpeed = std::min(state.currentSpeed, targetSpeed);
    } else if (state.currentSpeed > targetSpeed) {
        state.currentSpeed -= adaptiveDecel * deltaTime;
        state.currentSpeed = std::max(state.currentSpeed, targetSpeed);
    }
    
    return state.currentSpeed;
}

bool ViewTransitionManager::processTransitionQueue() {
    if (m_transitionQueue.empty()) return false;
    
    // Sort active transitions by priority
    std::vector<std::reference_wrapper<const TransitionState>> activeByPriority;
    for (const auto& transition : m_activeTransitions) {
        if (transition.active) {
            activeByPriority.push_back(std::ref(transition));
        }
    }
    std::sort(activeByPriority.begin(), activeByPriority.end(),
        [](const TransitionState& a, const TransitionState& b) {
            return a.params.priority > b.params.priority;
        });
    
    // Process queue based on priorities and states
    auto& queued = m_transitionQueue.front();
    bool canStart = true;
    
    if (!activeByPriority.empty()) {
        const auto& highestPriority = activeByPriority.front().get();
        
        if (highestPriority.params.priority > queued.params.priority) {
            // Higher priority transition is active
            canStart = false;
        } else if (highestPriority.params.priority == queued.params.priority) {
            // Equal priority - check progress
            if (highestPriority.progress < 0.8f) {
                // Let current transition get closer to completion
                canStart = false;
            }
        }
    }
    
    if (canStart) {
        // Find current view state
        ViewState currentState;
        if (!m_activeTransitions.empty()) {
            // Find the most relevant active transition for current state
            const TransitionState* relevantTransition = nullptr;
            float highestProgress = 0.0f;
            
            for (const auto& transition : m_activeTransitions) {
                if (transition.active && transition.progress > highestProgress) {
                    relevantTransition = &transition;
                    highestProgress = transition.progress;
                }
            }
            
            if (relevantTransition) {
                currentState = interpolateStates(
                    relevantTransition->start,
                    relevantTransition->end,
                    relevantTransition->progress,
                    *relevantTransition
                );
            }
        }
        
        // Cancel lower priority active transitions
        for (auto& transition : m_activeTransitions) {
            if (transition.active && transition.params.priority < queued.params.priority) {
                transition.active = false;
            }
        }
        
        // Start the queued transition
        startTransition(currentState, queued.targetState, queued.params);
        m_transitionQueue.pop();
        return true;
    }
    
    return false;
}

uint64_t ViewTransitionManager::startTransition(
    const ViewState& start,
    const ViewState& end,
    const TransitionParams& params
) {
    // Check if we should queue this transition
    if (params.queueIfBlocked) {
        for (const auto& transition : m_activeTransitions) {
            if (transition.active && transition.params.priority > params.priority) {
                QueuedTransition queued{end, params, m_nextTransitionId++};
                m_transitionQueue.push(std::move(queued));
                return queued.queueTime;
            }
        }
    }

    // Generate unique ID for this transition
    uint64_t transitionId = m_nextTransitionId++;

    TransitionState state{
        .id = transitionId,
        .start = start,
        .end = end,
        .params = params,
        .progress = 0.0f,
        .pathPoints = {},
        .startTime = 0.0f,
        .active = true,
        .currentSpeed = params.accelParams ? params.accelParams->initialSpeed : 1.0f,
        .pathDistance = 0.0f,
        .currentWaypointIndex = 0,
        .waypointTimer = 0.0f,
        .needsPathRegeneration = false
    };

    // Generate path based on parameters
    if (params.usePathPlanning) {
        if (params.pathParams) {
            state.controlPoints = generateBezierPath(
                start.position,
                end.position,
                *params.pathParams
            );
            
            // Pre-calculate optimized path points
            const size_t numPoints = 100;
            state.optimizedPath.reserve(numPoints);
            for (size_t i = 0; i < numPoints; ++i) {
                float t = static_cast<float>(i) / (numPoints - 1);
                state.optimizedPath.push_back(
                    evaluateBezierCurve(state.controlPoints, t)
                );
            }
            
            // Calculate total path distance
            state.pathDistance = 0.0f;
            for (size_t i = 1; i < state.optimizedPath.size(); ++i) {
                state.pathDistance += glm::distance(
                    state.optimizedPath[i],
                    state.optimizedPath[i - 1]
                );
            }
        } else {
            state.pathPoints = generatePath(start.position, end.position);
        }
    }

    // Remove any lower priority transitions
    m_activeTransitions.erase(
        std::remove_if(
            m_activeTransitions.begin(),
            m_activeTransitions.end(),
            [&params](const TransitionState& existing) {
                return !existing.active || existing.params.priority < params.priority;
            }
        ),
        m_activeTransitions.end()
    );

    m_activeTransitions.push_back(std::move(state));
    return transitionId;
}

void ViewTransitionManager::update(float deltaTime) {
    // Process queued transitions
    processTransitionQueue();

    // Update all active transitions
    for (auto& state : m_activeTransitions) {
        if (state.active) {
            bool complete = updateTransition(state, deltaTime);
            
            if (complete) {
                state.active = false;
                if (m_completionCallback) {
                    m_completionCallback(state.id);
                }
            }
        }
    }

    // Clean up completed transitions
    m_activeTransitions.erase(
        std::remove_if(
            m_activeTransitions.begin(),
            m_activeTransitions.end(),
            [](const TransitionState& state) {
                return !state.active;
            }
        ),
        m_activeTransitions.end()
    );
}

bool ViewTransitionManager::cancelTransition(uint64_t transitionId) {
    auto it = std::find_if(
        m_activeTransitions.begin(),
        m_activeTransitions.end(),
        [transitionId](const TransitionState& state) {
            return state.id == transitionId && state.active;
        }
    );

    if (it != m_activeTransitions.end()) {
        it->active = false;
        return true;
    }

    return false;
}

uint64_t ViewTransitionManager::interruptWithNewTransition(
    const ViewState& newEnd,
    const TransitionParams& params
) {
    // Find the highest priority active transition
    auto it = std::max_element(
        m_activeTransitions.begin(),
        m_activeTransitions.end(),
        [](const TransitionState& a, const TransitionState& b) {
            return a.params.priority < b.params.priority;
        }
    );

    if (it != m_activeTransitions.end() && it->active) {
        // Use current interpolated state as start
        ViewState currentState = interpolateStates(
            it->start,
            it->end,
            it->progress,
            *it
        );

        // Cancel current transition
        it->active = false;

        // Start new transition from current state
        return startTransition(currentState, newEnd, params);
    }

    // No active transition to interrupt, start new one
    return 0;
}

void ViewTransitionManager::setTransitionCompletedCallback(
    std::function<void(uint64_t)> callback
) {
    m_completionCallback = std::move(callback);
}

void ViewTransitionManager::setProgressCallback(
    std::function<void(uint64_t, float)> callback
) {
    m_progressCallback = std::move(callback);
}

std::vector<glm::vec3> ViewTransitionManager::generatePath(
    const glm::vec3& start,
    const glm::vec3& end
) {
    std::vector<glm::vec3> path;
    path.push_back(start);

    // Create a temporary viewport for collision testing
    Viewport tempViewport;
    const glm::vec3 up(0.0f, 1.0f, 0.0f); // Default up vector
    tempViewport.setView(start, end, up);
    
    // Use camera collision system to find intermediate points
    glm::vec3 current = start;
    while (glm::distance(current, end) > 0.1f) {
        glm::vec3 direction = glm::normalize(end - current);
        float step = std::min(1.0f, glm::distance(current, end));
        
        glm::vec3 nextPoint = current + direction * step;
        
        // Update viewport position for collision testing
        tempViewport.setView(current, nextPoint, up);
        
        // Check for potential collisions
        if (m_collisionSystem->predictCollision(tempViewport, *m_sceneGraph, nextPoint)) {
            // Resolve collision to find safe point
            nextPoint = m_collisionSystem->resolveCollision(tempViewport, *m_sceneGraph, nextPoint);
        }
        
        path.push_back(nextPoint);
        current = nextPoint;
    }

    path.push_back(end);
    return path;
}

ViewState ViewTransitionManager::interpolateStates(
    const ViewState& start,
    const ViewState& end,
    float t,
    const TransitionState& state
) const {
    ViewState result;

    // Position interpolation (using path if available)
    if (!state.optimizedPath.empty()) {
        // Use pre-calculated optimized path
        size_t index = static_cast<size_t>(t * (state.optimizedPath.size() - 1));
        result.position = state.optimizedPath[index];
    } else if (!state.pathPoints.empty()) {
        // Use basic path points
        size_t index = static_cast<size_t>(t * (state.pathPoints.size() - 1));
        result.position = state.pathPoints[index];
    } else {
        // Direct linear interpolation
        result.position = glm::mix(start.position, end.position, t);
    }

    // Handle up vector based on maintainUpVector setting
    if (start.upVector && end.upVector) {
        result.upVector = state.params.maintainUpVector 
            ? start.upVector  // Keep consistent up vector
            : std::optional<glm::vec3>(glm::mix(*start.upVector, *end.upVector, t));
    } else if (start.upVector) {
        result.upVector = start.upVector;
    } else if (end.upVector) {
        result.upVector = end.upVector;
    }

    // Handle motion speed
    if (start.motionSpeed && end.motionSpeed) {
        result.motionSpeed = std::optional<float>(
            glm::mix(*start.motionSpeed, *end.motionSpeed, t));
    } else if (start.motionSpeed) {
        result.motionSpeed = start.motionSpeed;
    } else if (end.motionSpeed) {
        result.motionSpeed = end.motionSpeed;
    }

    // Handle look-ahead point if enabled
    if (state.params.lookAheadEnabled) {
        // Calculate look-ahead point based on current position and path
        if (!state.optimizedPath.empty()) {
            size_t currentIndex = static_cast<size_t>(t * (state.optimizedPath.size() - 1));
            size_t lookAheadIndex = std::min(
                currentIndex + 5,  // Look 5 points ahead
                state.optimizedPath.size() - 1
            );
            result.lookAheadPoint = std::optional<glm::vec3>(
                state.optimizedPath[lookAheadIndex]);
        }
    }

    // Rotation interpolation using quaternion slerp
    result.rotation = glm::slerp(start.rotation, end.rotation, t);

    // Linear interpolation for scalar values
    result.fieldOfView = glm::mix(start.fieldOfView, end.fieldOfView, t);
    result.orthographicScale = glm::mix(start.orthographicScale, end.orthographicScale, t);

    // Target interpolation
    result.target = glm::mix(start.target, end.target, t);

    // Projection type changes at midpoint
    result.projectionType = t < 0.5f ? start.projectionType : end.projectionType;

    return result;
}

bool ViewTransitionManager::updateTransition(
    TransitionState& state,
    float deltaTime
) {
    // Handle waypoint dwell time if using path planning
    if (state.params.pathParams && !state.params.pathParams->waypoints.empty()) {
        const auto& waypoints = state.params.pathParams->waypoints;
        if (state.currentWaypointIndex < waypoints.size()) {
            state.waypointTimer += deltaTime;
            if (state.waypointTimer < waypoints[state.currentWaypointIndex].dwellTime) {
                return false;
            }
            state.currentWaypointIndex++;
            state.waypointTimer = 0.0f;
        }
    }

    // Update speed based on acceleration parameters
    float currentSpeed = updateTransitionSpeed(state, deltaTime);
    
    // Calculate progress increment based on speed and path distance
    float progressIncrement;
    if (state.pathDistance > 0.0f && state.params.accelParams) {
        progressIncrement = (currentSpeed * deltaTime) / state.pathDistance;
    } else {
        progressIncrement = deltaTime / state.params.duration;
    }
    
    // Update progress
    state.progress = std::min(1.0f, state.progress + progressIncrement);

    // Apply easing function if provided
    float easedProgress = state.params.easingFunc
        ? state.params.easingFunc(state.progress)
        : state.progress;

    // Check for path regeneration
    if (state.needsPathRegeneration) {
        if (state.params.pathParams) {
            state.controlPoints = generateBezierPath(
                state.start.position,
                state.end.position,
                *state.params.pathParams
            );
        }
        state.needsPathRegeneration = false;
    }

    // Notify progress
    if (m_progressCallback) {
        m_progressCallback(state.id, easedProgress);
    }

    return state.progress >= 1.0f;
}

} // namespace Graphics
} // namespace RebelCAD
