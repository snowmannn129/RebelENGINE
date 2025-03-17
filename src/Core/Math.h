#pragma once

#include <cmath>
#include <algorithm>
#include <array>
#include <stdexcept>

// Represents an axis-aligned bounding box for culling
struct AABB {
    Vector3 min;
    Vector3 max;

    AABB() : min(Vector3(0,0,0)), max(Vector3(0,0,0)) {}
    AABB(const Vector3& min, const Vector3& max) : min(min), max(max) {}
    
    // Expand this AABB to include another AABB
    void expand(const AABB& other) {
        min = Vector3(
            std::min(min.x, other.min.x),
            std::min(min.y, other.min.y),
            std::min(min.z, other.min.z)
        );
        max = Vector3(
            std::max(max.x, other.max.x),
            std::max(max.y, other.max.y),
            std::max(max.z, other.max.z)
        );
    }
    
    bool intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }
    
    bool isInFrustum(const std::array<Vector4, 6>& frustumPlanes) const {
        // Test against each frustum plane
        for (const auto& plane : frustumPlanes) {
            // Get the positive vertex (p-vertex) based on plane normal
            Vector3 p(
                plane.x > 0 ? max.x : min.x,
                plane.y > 0 ? max.y : min.y,
                plane.z > 0 ? max.z : min.z
            );

            // If p-vertex is outside, whole AABB is outside
            if (plane.x * p.x + plane.y * p.y + plane.z * p.z + plane.w < 0) {
                return false;
            }
        }
        return true;
    }
};

struct Vector2 {
    float x, y;
    
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
    
    float Length() const { return std::sqrt(x * x + y * y); }
    void Normalize() {
        float len = Length();
        if (len > 0) {
            x /= len;
            y /= len;
        }
    }
};

struct Vector3 {
    float x, y, z;
    
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    
    // Operator overloads
    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    
    Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    
    Vector3& operator-=(const Vector3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    
    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    
    Vector3& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    
    Vector3 operator/(float scalar) const {
        if (scalar != 0) {
            float inv = 1.0f / scalar;
            return Vector3(x * inv, y * inv, z * inv);
        }
        return *this;
    }

    Vector3 operator-() const {
        return Vector3(-x, -y, -z);
    }

    bool operator==(const Vector3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    float& operator[](size_t i) {
        switch(i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw std::out_of_range("Vector3 index out of range");
        }
    }

    const float& operator[](size_t i) const {
        switch(i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw std::out_of_range("Vector3 index out of range");
        }
    }

    void Normalize() {
        float len = Length();
        if (len > 0) {
            x /= len;
            y /= len;
            z /= len;
        }
    }

    // Static utility functions
    static float Dot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vector3 Cross(const Vector3& a, const Vector3& b) {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
};

struct Vector4 {
    float x, y, z, w;
    
    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    
    float Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    
    Vector4 operator*(float scalar) const {
        return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
    }
    
    void Normalize() {
        float len = Length();
        if (len > 0) {
            x /= len;
            y /= len;
            z /= len;
            w /= len;
        }
    }
};

struct Matrix4 {
    float m[16];
    
    Matrix4() {
        for (int i = 0; i < 16; i++) m[i] = 0;
        m[0] = m[5] = m[10] = m[15] = 1;
    }

    // Array access operators for convenient indexing
    float& operator[](size_t i) { return m[i]; }
    const float& operator[](size_t i) const { return m[i]; }
    
    // Matrix multiplication
    Matrix4 operator*(const Matrix4& other) const {
        Matrix4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i * 4 + j] = 
                    m[i * 4 + 0] * other.m[0 * 4 + j] +
                    m[i * 4 + 1] * other.m[1 * 4 + j] +
                    m[i * 4 + 2] * other.m[2 * 4 + j] +
                    m[i * 4 + 3] * other.m[3 * 4 + j];
            }
        }
        return result;
    }

    // Transform a vector
    Vector4 operator*(const Vector4& v) const {
        return Vector4(
            m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w,
            m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7] * v.w,
            m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11] * v.w,
            m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w
        );
    }

    Vector3 TransformPoint(const Vector3& v) const {
        Vector4 result = (*this) * Vector4(v.x, v.y, v.z, 1.0f);
        if (result.w != 0.0f) {
            float invW = 1.0f / result.w;
            return Vector3(result.x * invW, result.y * invW, result.z * invW);
        }
        return Vector3(result.x, result.y, result.z);
    }

    // Extract the translation component from the transformation matrix
    Vector3 GetTranslation() const {
        return Vector3(m[3], m[7], m[11]);
    }

    // Static factory methods
    static Matrix4 Identity() {
        return Matrix4();
    }

    static Matrix4 Translation(const Vector3& v) {
        Matrix4 result;
        result.m[3] = v.x;
        result.m[7] = v.y;
        result.m[11] = v.z;
        return result;
    }

    static Matrix4 Scale(const Vector3& v) {
        Matrix4 result;
        result.m[0] = v.x;
        result.m[5] = v.y;
        result.m[10] = v.z;
        return result;
    }

    static Matrix4 RotationX(float radians) {
        Matrix4 result;
        float c = std::cos(radians);
        float s = std::sin(radians);
        result.m[5] = c;
        result.m[6] = -s;
        result.m[9] = s;
        result.m[10] = c;
        return result;
    }

    static Matrix4 RotationY(float radians) {
        Matrix4 result;
        float c = std::cos(radians);
        float s = std::sin(radians);
        result.m[0] = c;
        result.m[2] = s;
        result.m[8] = -s;
        result.m[10] = c;
        return result;
    }

    static Matrix4 RotationZ(float radians) {
        Matrix4 result;
        float c = std::cos(radians);
        float s = std::sin(radians);
        result.m[0] = c;
        result.m[1] = -s;
        result.m[4] = s;
        result.m[5] = c;
        return result;
    }

    // Perspective projection matrix
    static Matrix4 Perspective(float fovY, float aspect, float nearZ, float farZ) {
        Matrix4 result;
        float tanHalfFovY = std::tan(fovY / 2);
        result.m[0] = 1 / (aspect * tanHalfFovY);
        result.m[5] = 1 / tanHalfFovY;
        result.m[10] = -(farZ + nearZ) / (farZ - nearZ);
        result.m[11] = -(2 * farZ * nearZ) / (farZ - nearZ);
        result.m[14] = -1;
        result.m[15] = 0;
        return result;
    }

    // Look-at view matrix
    static Matrix4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
        Vector3 f = target - eye;
        f.Normalize();
        Vector3 s = Vector3(
            f.y * up.z - f.z * up.y,
            f.z * up.x - f.x * up.z,
            f.x * up.y - f.y * up.x
        );
        s.Normalize();
        Vector3 u = Vector3(
            s.y * f.z - s.z * f.y,
            s.z * f.x - s.x * f.z,
            s.x * f.y - s.y * f.x
        );

        Matrix4 result;
        result.m[0] = s.x;
        result.m[1] = u.x;
        result.m[2] = -f.x;
        result.m[3] = -Vector3(s.x, u.x, -f.x).Length();
        result.m[4] = s.y;
        result.m[5] = u.y;
        result.m[6] = -f.y;
        result.m[7] = -Vector3(s.y, u.y, -f.y).Length();
        result.m[8] = s.z;
        result.m[9] = u.z;
        result.m[10] = -f.z;
        result.m[11] = -Vector3(s.z, u.z, -f.z).Length();
        return result;
    }

    // Extract frustum planes (for culling)
    std::array<Vector4, 6> ExtractFrustumPlanes() const {
        std::array<Vector4, 6> planes = {
            // Left plane
            Vector4(
            m[3] + m[0], m[7] + m[4],
            m[11] + m[8], m[15] + m[12]
            ),
            // Right plane
            Vector4(
            m[3] - m[0], m[7] - m[4],
            m[11] - m[8], m[15] - m[12]
            ),
            // Bottom plane
            Vector4(
            m[3] + m[1], m[7] + m[5],
            m[11] + m[9], m[15] + m[13]
            ),
            // Top plane
            Vector4(
            m[3] - m[1], m[7] - m[5],
            m[11] - m[9], m[15] - m[13]
            ),
            // Near plane
            Vector4(
            m[3] + m[2], m[7] + m[6],
            m[11] + m[10], m[15] + m[14]
            ),
            // Far plane
            Vector4(
            m[3] - m[2], m[7] - m[6],
            m[11] - m[10], m[15] - m[14]
            )
        };

        // Normalize all planes
        for (auto& plane : planes) {
            float len = std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
            if (len > 0) {
                plane = plane * (1.0f / len);
            }
        }
        
        return planes;
    }
};

namespace Colors {
    const Vector4 Green(0.0f, 1.0f, 0.0f, 1.0f);
    const Vector4 Yellow(1.0f, 1.0f, 0.0f, 1.0f);
}

namespace Math {
    const float PI = 3.14159265358979323846f;
    
    inline float RadToDeg(float rad) {
        return rad * 180.0f / PI;
    }
    
    inline float DegToRad(float deg) {
        return deg * PI / 180.0f;
    }
    
    template<typename T>
    inline T Clamp(T value, T min, T max) {
        return std::min(std::max(value, min), max);
    }
    
    inline float Lerp(float a, float b, float t) {
        return a + (b - a) * Clamp(t, 0.0f, 1.0f);
    }
}
