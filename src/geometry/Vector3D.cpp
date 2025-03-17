#include "modeling/Geometry.h"
#include <cmath>

namespace RebelCAD {
namespace Modeling {

double Vector3D::Magnitude() const {
    return std::sqrt(x*x + y*y + z*z);
}

Vector3D Vector3D::Normalize() const {
    double mag = Magnitude();
    if (mag < 1e-10) return Vector3D();
    return Vector3D(x/mag, y/mag, z/mag);
}

double Vector3D::Dot(const Vector3D& other) const {
    return x*other.x + y*other.y + z*other.z;
}

Vector3D Vector3D::Cross(const Vector3D& other) const {
    return Vector3D(
        y*other.z - z*other.y,
        z*other.x - x*other.z,
        x*other.y - y*other.x
    );
}

} // namespace Modeling
} // namespace RebelCAD
