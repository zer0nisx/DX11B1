#pragma once
#include <DirectXMath.h>
#include "Vector3.h"

namespace GameEngine {
namespace Math {

class Matrix4 {
public:
    DirectX::XMMATRIX m_matrix;

    // Constructors
    Matrix4() : m_matrix(DirectX::XMMatrixIdentity()) {}
    Matrix4(const DirectX::XMMATRIX& matrix) : m_matrix(matrix) {}

    // Static creation functions
    static Matrix4 Identity() {
        return Matrix4(DirectX::XMMatrixIdentity());
    }

    static Matrix4 Translation(const Vector3& translation) {
        return Matrix4(DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z));
    }

    static Matrix4 Translation(float x, float y, float z) {
        return Matrix4(DirectX::XMMatrixTranslation(x, y, z));
    }

    static Matrix4 RotationX(float angleRadians) {
        return Matrix4(DirectX::XMMatrixRotationX(angleRadians));
    }

    static Matrix4 RotationY(float angleRadians) {
        return Matrix4(DirectX::XMMatrixRotationY(angleRadians));
    }

    static Matrix4 RotationZ(float angleRadians) {
        return Matrix4(DirectX::XMMatrixRotationZ(angleRadians));
    }

    static Matrix4 RotationYawPitchRoll(float yaw, float pitch, float roll) {
        return Matrix4(DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
    }

    static Matrix4 Scale(const Vector3& scale) {
        return Matrix4(DirectX::XMMatrixScaling(scale.x, scale.y, scale.z));
    }

    static Matrix4 Scale(float x, float y, float z) {
        return Matrix4(DirectX::XMMatrixScaling(x, y, z));
    }

    static Matrix4 Scale(float uniformScale) {
        return Matrix4(DirectX::XMMatrixScaling(uniformScale, uniformScale, uniformScale));
    }

    // View and projection matrices
    static Matrix4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
        return Matrix4(DirectX::XMMatrixLookAtLH(
            eye.ToXMVECTOR(),
            target.ToXMVECTOR(),
            up.ToXMVECTOR()
        ));
    }

    static Matrix4 Perspective(float fovRadians, float aspectRatio, float nearPlane, float farPlane) {
        return Matrix4(DirectX::XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearPlane, farPlane));
    }

    static Matrix4 Orthographic(float width, float height, float nearPlane, float farPlane) {
        return Matrix4(DirectX::XMMatrixOrthographicLH(width, height, nearPlane, farPlane));
    }

    // Operators
    Matrix4 operator*(const Matrix4& other) const {
        return Matrix4(DirectX::XMMatrixMultiply(m_matrix, other.m_matrix));
    }

    Matrix4& operator*=(const Matrix4& other) {
        m_matrix = DirectX::XMMatrixMultiply(m_matrix, other.m_matrix);
        return *this;
    }

    // Utility functions
    Matrix4 Transpose() const {
        return Matrix4(DirectX::XMMatrixTranspose(m_matrix));
    }

    Matrix4 Inverse() const {
        DirectX::XMVECTOR det;
        return Matrix4(DirectX::XMMatrixInverse(&det, m_matrix));
    }

    // Get components
    Vector3 GetTranslation() const {
        DirectX::XMFLOAT4X4 matrix;
        DirectX::XMStoreFloat4x4(&matrix, m_matrix);
        return Vector3(matrix._41, matrix._42, matrix._43);
    }

    Vector3 GetScale() const {
        DirectX::XMFLOAT4X4 matrix;
        DirectX::XMStoreFloat4x4(&matrix, m_matrix);

        Vector3 scale;
        scale.x = Vector3(matrix._11, matrix._12, matrix._13).Length();
        scale.y = Vector3(matrix._21, matrix._22, matrix._23).Length();
        scale.z = Vector3(matrix._31, matrix._32, matrix._33).Length();
        return scale;
    }

    // Convert to DirectX types
    DirectX::XMMATRIX ToXMMATRIX() const { return m_matrix; }

    DirectX::XMFLOAT4X4 ToXMFLOAT4X4() const {
        DirectX::XMFLOAT4X4 result;
        DirectX::XMStoreFloat4x4(&result, m_matrix);
        return result;
    }

    // Transform vector
    Vector3 TransformPoint(const Vector3& point) const {
        DirectX::XMVECTOR v = DirectX::XMVector3TransformCoord(point.ToXMVECTOR(), m_matrix);
        return Vector3(v);
    }

    Vector3 TransformDirection(const Vector3& direction) const {
        DirectX::XMVECTOR v = DirectX::XMVector3TransformNormal(direction.ToXMVECTOR(), m_matrix);
        return Vector3(v);
    }
};

} // namespace Math
} // namespace GameEngine
