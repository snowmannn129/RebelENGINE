#include "Graphics/Frustum.h"
#include <glm/gtc/matrix_access.hpp>

namespace RebelCAD {
namespace Graphics {

Frustum::Frustum(const glm::mat4& viewProj) {
    update(viewProj);
}

void Frustum::update(const glm::mat4& viewProj) {
    // Extract planes from view-projection matrix
    // Each plane is defined by a row of the view-projection matrix
    // after combining certain rows and columns
    
    // Left plane
    planes_[LEFT] = extractPlane(viewProj, 0);
    
    // Right plane
    planes_[RIGHT] = extractPlane(viewProj, 1);
    
    // Bottom plane
    planes_[BOTTOM] = extractPlane(viewProj, 2);
    
    // Top plane
    planes_[TOP] = extractPlane(viewProj, 3);
    
    // Near plane
    planes_[NEAR] = extractPlane(viewProj, 4);
    
    // Far plane
    planes_[FAR] = extractPlane(viewProj, 5);
}

Frustum::Plane Frustum::extractPlane(const glm::mat4& viewProj, int row) {
    Plane plane;
    
    // Get the appropriate coefficients based on the row
    glm::vec4 coefficients;
    switch (row) {
        case 0: // Left plane
            coefficients = glm::row(viewProj, 3) + glm::row(viewProj, 0);
            break;
        case 1: // Right plane
            coefficients = glm::row(viewProj, 3) - glm::row(viewProj, 0);
            break;
        case 2: // Bottom plane
            coefficients = glm::row(viewProj, 3) + glm::row(viewProj, 1);
            break;
        case 3: // Top plane
            coefficients = glm::row(viewProj, 3) - glm::row(viewProj, 1);
            break;
        case 4: // Near plane
            coefficients = glm::row(viewProj, 3) + glm::row(viewProj, 2);
            break;
        case 5: // Far plane
            coefficients = glm::row(viewProj, 3) - glm::row(viewProj, 2);
            break;
    }
    
    // Extract normal and distance
    plane.normal = glm::vec3(coefficients);
    plane.distance = coefficients.w;
    
    // Normalize the plane
    float length = glm::length(plane.normal);
    plane.normal /= length;
    plane.distance /= length;
    
    return plane;
}

bool Frustum::containsPoint(const glm::vec3& point) const {
    // Point is inside if it's on the positive side of all planes
    for (const auto& plane : planes_) {
        if (!plane.isOnPositiveSide(point)) {
            return false;
        }
    }
    return true;
}

bool Frustum::intersectsSphere(const glm::vec3& center, float radius) const {
    // Sphere intersects if its center is within radius distance of all planes
    for (const auto& plane : planes_) {
        float distance = glm::dot(plane.normal, center) + plane.distance;
        if (distance < -radius) {
            return false; // Sphere is completely behind this plane
        }
    }
    return true;
}

bool Frustum::intersectsAABB(const glm::vec3& min, const glm::vec3& max) const {
    // Test AABB against all frustum planes
    for (const auto& plane : planes_) {
        // Find the positive vertex (p-vertex) - the vertex of the box most along the plane normal
        glm::vec3 p(
            plane.normal.x >= 0 ? max.x : min.x,
            plane.normal.y >= 0 ? max.y : min.y,
            plane.normal.z >= 0 ? max.z : min.z
        );
        
        // Find the negative vertex (n-vertex) - the vertex of the box most against the plane normal
        glm::vec3 n(
            plane.normal.x >= 0 ? min.x : max.x,
            plane.normal.y >= 0 ? min.y : max.y,
            plane.normal.z >= 0 ? min.z : max.z
        );
        
        // If the negative vertex is outside, the box is completely outside
        if (!plane.isOnPositiveSide(n)) {
            return false;
        }
    }
    
    return true;
}

} // namespace Graphics
} // namespace RebelCAD
