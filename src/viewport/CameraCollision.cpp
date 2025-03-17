#include "Graphics/CameraCollision.h"
#include "Graphics/SpatialPartitioning.h"
#include <glm/gtx/norm.hpp>
#include <algorithm>

namespace RebelCAD {
namespace Graphics {

CameraCollision::CollisionResult CameraCollision::testCollision(const Viewport& viewport, const SceneGraph& scene) const {
    const glm::vec3& cameraPos = viewport.getEyePosition();
    const glm::vec3& target = viewport.getTarget();
    
    // Get direction from target to camera for reverse sphere cast
    glm::vec3 direction = glm::normalize(cameraPos - target);
    
    // Perform sphere cast from target towards camera
    return sphereCast(target, direction, config_.probeRadius, scene);
}

glm::vec3 CameraCollision::resolveCollision(const Viewport& viewport, const SceneGraph& scene,
                                          const glm::vec3& targetPos) const {
    const glm::vec3& currentPos = viewport.getEyePosition();
    const glm::vec3& lookTarget = viewport.getTarget();

    // Early exit if no movement
    if (glm::length2(currentPos - targetPos) < 1e-6f) {
        return currentPos;
    }

    // Initialize resolved position
    glm::vec3 resolvedPos = targetPos;
    
    // Iterative collision resolution
    for (int i = 0; i < config_.maxIterations; ++i) {
        // Test for collision at current resolved position
        CameraCollision::CollisionResult result = sphereCast(lookTarget, 
                                          glm::normalize(resolvedPos - lookTarget),
                                          config_.probeRadius, scene);
        
        if (!result.hasCollision) {
            break; // No collision, position is good
        }

        // Calculate offset direction based on collision normal
        glm::vec3 offset = result.contactNormal * 
                          (result.penetrationDepth + config_.minDistance);
        
        // Adjust position away from collision
        resolvedPos = smoothAdjustPosition(resolvedPos, targetPos,
                                         result.contactPoint, result.contactNormal);
        
        // Ensure minimum distance is maintained
        float distToTarget = glm::distance(resolvedPos, lookTarget);
        if (distToTarget < config_.minDistance) {
            resolvedPos = lookTarget + glm::normalize(resolvedPos - lookTarget) * 
                         config_.minDistance;
        }
    }

    return resolvedPos;
}

bool CameraCollision::predictCollision(const Viewport& viewport, const SceneGraph& scene,
                                     const glm::vec3& targetPos, int steps) const {
    if (!config_.enablePrediction) {
        return false;
    }

    const glm::vec3& startPos = viewport.getEyePosition();
    const glm::vec3& lookTarget = viewport.getTarget();

    // Check points along movement path
    for (int i = 1; i <= steps; ++i) {
        float t = static_cast<float>(i) / steps;
        glm::vec3 testPos = glm::mix(startPos, targetPos, t);
        
        // Test for collision at interpolated position
        CameraCollision::CollisionResult result = sphereCast(lookTarget,
                                          glm::normalize(testPos - lookTarget),
                                          config_.probeRadius, scene);
        
        if (result.hasCollision) {
            return true;
        }
    }

    return false;
}

CameraCollision::CollisionResult CameraCollision::sphereCast(const glm::vec3& origin,
                                          const glm::vec3& direction,
                                          float radius,
                                          const SceneGraph& scene) const {
    CameraCollision::CollisionResult result;
    
    // Use scene's spatial partitioning to efficiently find potential collisions
    auto potentialCollisions = scene.getSpatialIndex().findIntersectingObjects(
        origin, direction, radius);
    
    float closestDist = std::numeric_limits<float>::max();
    
    // Test against each potential collision object
    for (const auto& obj : potentialCollisions) {
        // Get object's bounding geometry
        auto geometry = obj->getBoundingGeometry();
        
        // Perform sphere-geometry intersection test
        auto intersection = geometry.intersectSphere(origin, direction, radius);
        
        if (intersection.hasCollision && intersection.distance < closestDist) {
            closestDist = intersection.distance;
            result.hasCollision = true;
            result.contactPoint = intersection.point;
            result.contactNormal = intersection.normal;
            result.penetrationDepth = intersection.penetration;
        }
    }
    
    return result;
}

std::pair<glm::vec3, glm::vec3> CameraCollision::findClosestPoint(
    const glm::vec3& cameraPos, const SceneGraph& scene) const {
    
    // Use spatial index to find nearby objects efficiently
    auto nearbyObjects = scene.getSpatialIndex().findNearbyObjects(
        cameraPos, config_.probeRadius * 2.0f);
    
    float closestDist = std::numeric_limits<float>::max();
    glm::vec3 closestPoint;
    glm::vec3 closestNormal;
    
    // Find closest point among nearby objects
    for (const auto& obj : nearbyObjects) {
        auto geometry = obj->getBoundingGeometry();
        auto [point, normal] = geometry.findClosestPoint(cameraPos);
        
        float dist = glm::length2(cameraPos - point);
        if (dist < closestDist) {
            closestDist = dist;
            closestPoint = point;
            closestNormal = normal;
        }
    }
    
    return {closestPoint, closestNormal};
}

glm::vec3 CameraCollision::smoothAdjustPosition(const glm::vec3& currentPos,
                                              const glm::vec3& targetPos,
                                              const glm::vec3& collisionPoint,
                                              const glm::vec3& normal) const {
    // Calculate offset vector
    glm::vec3 offset = normal * (config_.minDistance + config_.probeRadius);
    
    // Project movement direction onto collision plane
    glm::vec3 moveDir = targetPos - currentPos;
    glm::vec3 projectedDir = moveDir - 
                            glm::dot(moveDir, normal) * normal;
    
    // Calculate adjusted target position
    glm::vec3 adjustedPos = collisionPoint + offset + 
                           projectedDir * config_.smoothingFactor;
    
    // Smoothly interpolate between current and adjusted position
    return glm::mix(currentPos, adjustedPos, config_.smoothingFactor);
}

} // namespace Graphics
} // namespace RebelCAD
