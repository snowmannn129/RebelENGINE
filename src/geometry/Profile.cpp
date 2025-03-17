#include "modeling/Profile.h"
#include <algorithm>
#include <stdexcept>

namespace rebel_cad {
namespace modeling {

Profile::Profile() = default;
Profile::~Profile() = default;

bool Profile::setBoundary(const std::vector<glm::vec2>& boundary, bool closed) {
    if (boundary.size() < 3) {
        return false;
    }

    boundary_.points = boundary;
    boundary_.closed = closed;

    // Ensure the boundary is properly oriented (counter-clockwise)
    orientBoundaries();
    return validateBoundary();
}

size_t Profile::addHole(const std::vector<glm::vec2>& hole, bool closed) {
    if (hole.size() < 3) {
        throw std::invalid_argument("Hole must have at least 3 points");
    }

    Curve hole_curve;
    hole_curve.points = hole;
    hole_curve.closed = closed;
    holes_.push_back(hole_curve);

    // Ensure holes are properly oriented (clockwise)
    orientBoundaries();
    return holes_.size() - 1;
}

void Profile::removeHole(size_t index) {
    if (index < holes_.size()) {
        holes_.erase(holes_.begin() + index);
    }
}

bool Profile::validate() const {
    if (!validateBoundary()) {
        return false;
    }

    if (!validateHoles()) {
        return false;
    }

    // Check for self-intersections
    if (checkSelfIntersections(boundary_)) {
        return false;
    }

    for (const auto& hole : holes_) {
        if (checkSelfIntersections(hole)) {
            return false;
        }
    }

    // Check hole placement
    for (const auto& hole : holes_) {
        // Check if hole is inside boundary
        if (!containsPoint(hole.points[0])) {
            return false;
        }

        // Check if holes intersect with each other
        for (const auto& other_hole : holes_) {
            if (&hole != &other_hole) {
                // TODO: Implement hole-hole intersection check
            }
        }
    }

    return true;
}

bool Profile::validateBoundary() const {
    if (boundary_.points.size() < 3) {
        return false;
    }

    // Check for duplicate points
    for (size_t i = 0; i < boundary_.points.size(); ++i) {
        size_t next = (i + 1) % boundary_.points.size();
        if (glm::distance(boundary_.points[i], boundary_.points[next]) < 1e-6f) {
            return false;
        }
    }

    return true;
}

bool Profile::validateHoles() const {
    for (const auto& hole : holes_) {
        if (hole.points.size() < 3) {
            return false;
        }

        // Check for duplicate points in holes
        for (size_t i = 0; i < hole.points.size(); ++i) {
            size_t next = (i + 1) % hole.points.size();
            if (glm::distance(hole.points[i], hole.points[next]) < 1e-6f) {
                return false;
            }
        }
    }

    return true;
}

bool Profile::checkSelfIntersections(const Curve& curve) const {
    // Simple O(n²) line segment intersection check
    for (size_t i = 0; i < curve.points.size(); ++i) {
        size_t i_next = (i + 1) % curve.points.size();
        for (size_t j = i + 2; j < curve.points.size(); ++j) {
            size_t j_next = (j + 1) % curve.points.size();
            
            // Skip adjacent segments
            if (i == 0 && j_next == 0) continue;
            
            // Check intersection
            glm::vec2 p1 = curve.points[i];
            glm::vec2 p2 = curve.points[i_next];
            glm::vec2 p3 = curve.points[j];
            glm::vec2 p4 = curve.points[j_next];
            
            // Line segment intersection test
            glm::vec2 s1 = p2 - p1;
            glm::vec2 s2 = p4 - p3;
            
            float s = (-s1.y * (p1.x - p3.x) + s1.x * (p1.y - p3.y)) / 
                     (-s2.x * s1.y + s1.x * s2.y);
            float t = (s2.x * (p1.y - p3.y) - s2.y * (p1.x - p3.x)) / 
                     (-s2.x * s1.y + s1.x * s2.y);
            
            if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
                return true;  // Intersection found
            }
        }
    }
    return false;
}

void Profile::orientBoundaries() {
    // Compute signed area to determine orientation
    auto computeSignedArea = [](const std::vector<glm::vec2>& points) -> float {
        float area = 0.0f;
        for (size_t i = 0; i < points.size(); ++i) {
            size_t j = (i + 1) % points.size();
            area += points[i].x * points[j].y - points[j].x * points[i].y;
        }
        return area / 2.0f;
    };

    // Ensure boundary is counter-clockwise (positive area)
    float boundary_area = computeSignedArea(boundary_.points);
    if (boundary_area < 0) {
        std::reverse(boundary_.points.begin(), boundary_.points.end());
    }

    // Ensure holes are clockwise (negative area)
    for (auto& hole : holes_) {
        float hole_area = computeSignedArea(hole.points);
        if (hole_area > 0) {
            std::reverse(hole.points.begin(), hole.points.end());
        }
    }
}

std::vector<glm::vec2> Profile::triangulate() const {
    std::vector<glm::vec2> triangles;
    
    // TODO: Implement proper triangulation with holes
    // For now, just create a simple fan triangulation of the boundary
    if (boundary_.points.size() >= 3) {
        const glm::vec2& center = boundary_.points[0];
        for (size_t i = 1; i < boundary_.points.size() - 1; ++i) {
            triangles.push_back(center);
            triangles.push_back(boundary_.points[i]);
            triangles.push_back(boundary_.points[i + 1]);
        }
    }
    
    return triangles;
}

void Profile::discretize(float tolerance) {
    // TODO: Implement curve discretization
    // This will be important for curved profiles
}

void Profile::transform(const glm::mat3& matrix) {
    // Transform boundary points
    for (auto& point : boundary_.points) {
        glm::vec3 transformed = matrix * glm::vec3(point.x, point.y, 1.0f);
        point = glm::vec2(transformed.x / transformed.z, transformed.y / transformed.z);
    }

    // Transform hole points
    for (auto& hole : holes_) {
        for (auto& point : hole.points) {
            glm::vec3 transformed = matrix * glm::vec3(point.x, point.y, 1.0f);
            point = glm::vec2(transformed.x / transformed.z, transformed.y / transformed.z);
        }
    }
}

float Profile::computeArea() const {
    float area = 0.0f;

    // Compute boundary area
    for (size_t i = 0; i < boundary_.points.size(); ++i) {
        size_t j = (i + 1) % boundary_.points.size();
        area += boundary_.points[i].x * boundary_.points[j].y;
        area -= boundary_.points[j].x * boundary_.points[i].y;
    }
    area = std::abs(area) / 2.0f;

    // Subtract hole areas
    for (const auto& hole : holes_) {
        float hole_area = 0.0f;
        for (size_t i = 0; i < hole.points.size(); ++i) {
            size_t j = (i + 1) % hole.points.size();
            hole_area += hole.points[i].x * hole.points[j].y;
            hole_area -= hole.points[j].x * hole.points[i].y;
        }
        area -= std::abs(hole_area) / 2.0f;
    }

    return area;
}

bool Profile::containsPoint(const glm::vec2& point) const {
    // Ray casting algorithm
    bool inside = false;
    for (size_t i = 0; i < boundary_.points.size(); ++i) {
        size_t j = (i + 1) % boundary_.points.size();
        if (((boundary_.points[i].y > point.y) != (boundary_.points[j].y > point.y)) &&
            (point.x < (boundary_.points[j].x - boundary_.points[i].x) * 
             (point.y - boundary_.points[i].y) / (boundary_.points[j].y - boundary_.points[i].y) +
             boundary_.points[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}

} // namespace modeling
} // namespace rebel_cad
