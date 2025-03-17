#pragma once
#include <array>
#include <cmath>
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

namespace Math {

// Vector3 class that maps to gl-matrix vec3
class Vec3 {
public:
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    static Vec3 normalize(const Vec3& v) {
        float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        if (len > 0) {
            return Vec3(v.x / len, v.y / len, v.z / len);
        }
        return v;
    }

    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
};

// Quaternion class that maps to gl-matrix quat
class Quat {
public:
    float x, y, z, w;

    Quat() : x(0), y(0), z(0), w(1) {}
    Quat(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}

    // Convert quaternion to 4x4 rotation matrix (column-major order)
    std::array<float, 16> toMatrix() const {
        float xx = x * x;
        float xy = x * y;
        float xz = x * z;
        float xw = x * w;
        float yy = y * y;
        float yz = y * z;
        float yw = y * w;
        float zz = z * z;
        float zw = z * w;

        return {
            1 - 2 * (yy + zz),     2 * (xy - zw),     2 * (xz + yw),     0,
            2 * (xy + zw),     1 - 2 * (xx + zz),     2 * (yz - xw),     0,
            2 * (xz - yw),         2 * (yz + xw),     1 - 2 * (xx + yy), 0,
            0,                     0,                  0,                  1
        };
    }
};

} // namespace Math
