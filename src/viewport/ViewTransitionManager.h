#pragma once

#include "ViewportTypes.h"
#include "SceneGraph.h"
#include "CameraCollision.h"
#include "SpatialPartitioning.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <vector>
#include <functional>
#include <queue>
#include <array>
#include <optional>

namespace testing {
    class Test;
}

namespace RebelCAD {
namespace Graphics {

class ViewTransitionManagerTest;

// Forward declarations
class SceneGraph;
class CameraCollision;

/**
 * @brief Represents a cubic Bezier curve control point
 */
struct BezierControlPoint {
    glm::vec3 position;
    float influence;  // Influence radius for path adjustment
};

/**
 * @brief Represents a waypoint in a camera path
 */
struct PathWaypoint {
    glm::vec3 position;
    glm::quat rotation;
    float dwellTime;     // Time to pause at waypoint
    float blendRadius;   // Radius for smooth blending with next point
};

/**
 * @brief Advanced path planning parameters
 */
struct PathPlanningParams {
    std::vector<PathWaypoint> waypoints;
    float pathSmoothing;         // 0-1 smoothing factor
    float collisionPadding;      // Extra padding for collision avoidance
    float optimizationPasses;    // Number of optimization iterations
    bool adaptiveSpeed;          // Adjust speed based on path complexity
};

/**
 * @brief Acceleration curve parameters
 */
struct AccelerationParams {
    float initialSpeed;          // Starting speed
    float maxSpeed;             // Maximum transition speed
    float acceleration;         // Rate of speed increase
    float deceleration;         // Rate of speed decrease
    bool adaptToPath;           // Adjust speed based on path curvature
};

/**
 * @brief Extended view state with additional parameters for advanced transitions
 */
struct ViewState {
    glm::vec3 position;
    glm::quat rotation;
    float fieldOfView;
    glm::vec3 target;
    ViewportProjection projectionType;
    float orthographicScale;
    
    // Advanced state parameters
    std::optional<glm::vec3> upVector;        // Optional up vector override
    std::optional<float> motionSpeed;         // Optional speed override
    std::optional<glm::vec3> lookAheadPoint;  // Point to look ahead to during motion
};

/**
 * @brief Extended transition parameters with advanced features
 */
struct TransitionParams {
    float duration;                    // Duration in seconds
    bool usePathPlanning;             // Whether to use path planning for collision avoidance
    bool allowInterruption;           // Whether the transition can be interrupted
    std::function<float(float)> easingFunc; // Custom easing function
    int priority;                     // Transition priority (higher numbers = higher priority)
    
    // Advanced parameters
    std::optional<PathPlanningParams> pathParams;    // Optional path planning parameters
    std::optional<AccelerationParams> accelParams;   // Optional acceleration parameters
    bool maintainUpVector;            // Keep consistent up vector during transition
    bool lookAheadEnabled;            // Enable look-ahead during motion
    float blendingRadius;             // Radius for state blending
    bool queueIfBlocked;              // Queue transition if higher priority one is active
};

/**
 * @brief Manages smooth transitions between different view states
 * 
 * Handles interpolation of camera position, rotation, and other view parameters
 * with support for path planning, collision avoidance, and custom easing functions.
 */
class ViewTransitionManager {
public:
    /**
     * @brief Constructor
     * @param sceneGraph Reference to scene graph for collision detection
     * @param collisionSystem Reference to camera collision system
     */
    ViewTransitionManager(
        std::shared_ptr<SceneGraph> sceneGraph,
        std::shared_ptr<CameraCollision> collisionSystem
    );

    /**
     * @brief Starts a transition between two view states
     * @param start Starting view state
     * @param end Target view state
     * @param params Transition parameters
     * @return Unique ID for the transition
     */
    uint64_t startTransition(
        const ViewState& start,
        const ViewState& end,
        const TransitionParams& params
    );

    /**
     * @brief Updates all active transitions
     * @param deltaTime Time elapsed since last update in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Cancels an active transition
     * @param transitionId ID of transition to cancel
     * @return true if transition was found and cancelled
     */
    bool cancelTransition(uint64_t transitionId);

    /**
     * @brief Interrupts current transition and starts a new one
     * @param newEnd New target view state
     * @param params Transition parameters
     * @return Unique ID for the new transition
     */
    uint64_t interruptWithNewTransition(
        const ViewState& newEnd,
        const TransitionParams& params
    );

    /**
     * @brief Sets a callback for transition completion
     * @param callback Function to call when transition completes
     */
    void setTransitionCompletedCallback(
        std::function<void(uint64_t)> callback
    );

    /**
     * @brief Sets a callback for transition progress updates
     * @param callback Function to call with progress (0-1) updates
     */
    void setProgressCallback(
        std::function<void(uint64_t, float)> callback
    );

protected:
    struct TransitionState {
        uint64_t id;
        ViewState start;
        ViewState end;
        TransitionParams params;
        float progress;
        std::vector<glm::vec3> pathPoints;
        float startTime;
        bool active;
        
        // Advanced state tracking
        std::vector<BezierControlPoint> controlPoints;
        float currentSpeed;
        float pathDistance;
        size_t currentWaypointIndex;
        float waypointTimer;
        std::vector<glm::vec3> optimizedPath;
        bool needsPathRegeneration;
    };

    // Test helper methods
    ViewState getCurrentInterpolatedState() const {
        if (!m_activeTransitions.empty()) {
            const auto& transition = m_activeTransitions.front();
            return interpolateStates(
                transition.start,
                transition.end,
                transition.progress,
                transition
            );
        }
        return ViewState{};
    }

    float getCurrentSpeed() const {
        return !m_activeTransitions.empty() ? m_activeTransitions.front().currentSpeed : 0.0f;
    }

    float getCurrentProgress() const {
        return !m_activeTransitions.empty() ? m_activeTransitions.front().progress : 0.0f;
    }

    bool isTransitionActive(uint64_t id) const {
        for (const auto& transition : m_activeTransitions) {
            if (transition.id == id && transition.active) {
                return true;
            }
        }
        return false;
    }

    // Protected methods for testing
    ViewState interpolateStates(
        const ViewState& start,
        const ViewState& end,
        float t,
        const TransitionState& state
    ) const;

private:
    // Transition queue for handling blocked transitions
    struct QueuedTransition {
        ViewState targetState;
        TransitionParams params;
        uint64_t queueTime;
    };
    std::queue<QueuedTransition> m_transitionQueue;

    /**
     * @brief Generates a Bezier curve path with collision avoidance
     * @param start Starting position
     * @param end Target position
     * @param params Path planning parameters
     * @return Vector of control points for the Bezier curve
     */
    std::vector<BezierControlPoint> generateBezierPath(
        const glm::vec3& start,
        const glm::vec3& end,
        const PathPlanningParams& params
    );

    /**
     * @brief Optimizes a path for smoothness and collision avoidance
     * @param controlPoints Initial control points
     * @param params Path planning parameters
     * @return Optimized control points
     */
    std::vector<BezierControlPoint> optimizePath(
        const std::vector<BezierControlPoint>& controlPoints,
        const PathPlanningParams& params
    );

    /**
     * @brief Calculates the current position along a Bezier curve
     * @param controlPoints Bezier curve control points
     * @param t Curve parameter (0-1)
     * @return Position on curve
     */
    glm::vec3 evaluateBezierCurve(
        const std::vector<BezierControlPoint>& controlPoints,
        float t
    );

    /**
     * @brief Updates transition speed based on acceleration parameters
     * @param state Current transition state
     * @param deltaTime Time elapsed since last update
     * @return Updated speed
     */
    float updateTransitionSpeed(
        TransitionState& state,
        float deltaTime
    );

    /**
     * @brief Processes the transition queue
     * @return true if a new transition was started
     */
    bool processTransitionQueue();

    /**
     * @brief Generates a basic collision-free path between two points
     * @param start Starting position
     * @param end Target position
     * @return Vector of path points
     */
    std::vector<glm::vec3> generatePath(
        const glm::vec3& start,
        const glm::vec3& end
    );

    /**
     * @brief Updates a single transition
     * @param state Transition state to update
     * @param deltaTime Time elapsed since last update
     * @return true if transition is complete
     */
    bool updateTransition(TransitionState& state, float deltaTime);

    std::shared_ptr<SceneGraph> m_sceneGraph;
    std::shared_ptr<CameraCollision> m_collisionSystem;
    std::vector<TransitionState> m_activeTransitions;
    std::function<void(uint64_t)> m_completionCallback;
    std::function<void(uint64_t, float)> m_progressCallback;
    uint64_t m_nextTransitionId;

    // Advanced transition settings
    float m_pathSmoothingFactor;
    float m_collisionPaddingDefault;
    size_t m_optimizationPassesDefault;
    float m_speedAdjustmentFactor;
    float m_waypointBlendingDefault;
};

} // namespace Graphics
} // namespace RebelCAD
