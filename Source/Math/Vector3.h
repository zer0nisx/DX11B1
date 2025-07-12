#pragma once
#include <DirectXMath.h>
#include <cmath>

namespace GameEngine {
namespace Math {

class Vector3 {
public:
    float x, y, z;

    // Constructors
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(const DirectX::XMFLOAT3& v) : x(v.x), y(v.y), z(v.z) {}
    Vector3(const DirectX::XMVECTOR& v) {
        DirectX::XMFLOAT3 temp;
        DirectX::XMStoreFloat3(&temp, v);
        x = temp.x; y = temp.y; z = temp.z;
    }

    // Operators
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3 operator/(float scalar) const {
        return Vector3(x / scalar, y / scalar, z / scalar);
    }

    Vector3& operator+=(const Vector3& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& other) {
        x -= other.x; y -= other.y; z -= other.z;
        return *this;
    }

    Vector3& operator*=(float scalar) {
        x *= scalar; y *= scalar; z *= scalar;
        return *this;
    }

    // Utility functions
    float Length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    float LengthSquared() const {
        return x * x + y * y + z * z;
    }

    Vector3 Normalized() const {
        float len = Length();
        if (len > 0.0f) {
            return *this / len;
        }
        return Vector3(0, 0, 0);
    }

    void Normalize() {
        float len = Length();
        if (len > 0.0f) {
            x /= len; y /= len; z /= len;
        }
    }

    float Dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 Cross(const Vector3& other) const {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // Conversion to DirectX types
    DirectX::XMFLOAT3 ToXMFLOAT3() const {
        return DirectX::XMFLOAT3(x, y, z);
    }

    DirectX::XMVECTOR ToXMVECTOR() const {
        return DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(x, y, z));
    }

    // Static utility vectors
    static Vector3 Zero() { return Vector3(0, 0, 0); }
    static Vector3 One() { return Vector3(1, 1, 1); }
    static Vector3 Up() { return Vector3(0, 1, 0); }
    static Vector3 Forward() { return Vector3(0, 0, 1); }
    static Vector3 Right() { return Vector3(1, 0, 0); }
};

} // namespace Math
} // namespace GameEngine
