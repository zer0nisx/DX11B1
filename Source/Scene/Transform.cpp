#include "Transform.h"
#include "Entity.h"
#include "../Core/Logger.h"
#include <algorithm>

namespace GameEngine {
namespace Scene {

Transform::Transform()
    : m_localPosition(0.0f, 0.0f, 0.0f)
    , m_localRotation(0.0f, 0.0f, 0.0f)
    , m_localScale(1.0f, 1.0f, 1.0f)
    , m_localMatrix(DirectX::XMMatrixIdentity())
    , m_worldMatrix(DirectX::XMMatrixIdentity())
    , m_localMatrixDirty(true)
    , m_worldMatrixDirty(true)
    , m_isDirty(true)
{
}

void Transform::OnStart() {
    // Initialize matrices
    UpdateLocalMatrix();
    UpdateWorldMatrix();
}

// Position methods
void Transform::SetLocalPosition(const DirectX::XMFLOAT3& position) {
    m_localPosition = position;
    m_localMatrixDirty = true;
    MarkWorldMatrixDirty();
    MarkDirty();
}

void Transform::SetLocalPosition(float x, float y, float z) {
    SetLocalPosition(DirectX::XMFLOAT3(x, y, z));
}

DirectX::XMFLOAT3 Transform::GetWorldPosition() const {
    UpdateWorldMatrix();
    DirectX::XMFLOAT3 worldPos;
    DirectX::XMStoreFloat3(&worldPos, m_worldMatrix.r[3]);
    return worldPos;
}

void Transform::SetWorldPosition(const DirectX::XMFLOAT3& position) {
    Transform* parent = GetParent();
    if (parent) {
        // Convert world position to local position
        DirectX::XMVECTOR worldPos = DirectX::XMLoadFloat3(&position);
        DirectX::XMMATRIX invParentWorld = DirectX::XMMatrixInverse(nullptr, parent->GetWorldMatrix());
        DirectX::XMVECTOR localPos = DirectX::XMVector3Transform(worldPos, invParentWorld);
        DirectX::XMStoreFloat3(&m_localPosition, localPos);
    } else {
        m_localPosition = position;
    }

    m_localMatrixDirty = true;
    MarkWorldMatrixDirty();
    MarkDirty();
}

void Transform::SetWorldPosition(float x, float y, float z) {
    SetWorldPosition(DirectX::XMFLOAT3(x, y, z));
}

// Rotation methods
void Transform::SetLocalRotation(const DirectX::XMFLOAT3& rotation) {
    m_localRotation = rotation;
    m_localMatrixDirty = true;
    MarkWorldMatrixDirty();
    MarkDirty();
}

void Transform::SetLocalRotation(float x, float y, float z) {
    SetLocalRotation(DirectX::XMFLOAT3(x, y, z));
}

DirectX::XMFLOAT3 Transform::GetWorldRotation() const {
    UpdateWorldMatrix();
    return MatrixToEuler(m_worldMatrix);
}

void Transform::SetWorldRotation(const DirectX::XMFLOAT3& rotation) {
    Transform* parent = GetParent();
    if (parent) {
        // Convert world rotation to local rotation relative to parent
        DirectX::XMMATRIX worldRot = CreateRotationMatrix();
        DirectX::XMMATRIX invParentRot = DirectX::XMMatrixInverse(nullptr, CreateRotationMatrix());
        DirectX::XMMATRIX localRot = DirectX::XMMatrixMultiply(invParentRot, worldRot);
        m_localRotation = MatrixToEuler(localRot);
    } else {
        m_localRotation = rotation;
    }

    m_localMatrixDirty = true;
    MarkWorldMatrixDirty();
    MarkDirty();
}

// Scale methods
void Transform::SetLocalScale(const DirectX::XMFLOAT3& scale) {
    m_localScale = scale;
    m_localMatrixDirty = true;
    MarkWorldMatrixDirty();
    MarkDirty();
}

void Transform::SetLocalScale(float x, float y, float z) {
    SetLocalScale(DirectX::XMFLOAT3(x, y, z));
}

void Transform::SetLocalScale(float uniformScale) {
    SetLocalScale(DirectX::XMFLOAT3(uniformScale, uniformScale, uniformScale));
}

DirectX::XMFLOAT3 Transform::GetWorldScale() const {
    UpdateWorldMatrix();

    // Extract scale from world matrix
    DirectX::XMFLOAT3 scale;
    scale.x = DirectX::XMVectorGetX(DirectX::XMVector3Length(m_worldMatrix.r[0]));
    scale.y = DirectX::XMVectorGetX(DirectX::XMVector3Length(m_worldMatrix.r[1]));
    scale.z = DirectX::XMVectorGetX(DirectX::XMVector3Length(m_worldMatrix.r[2]));

    return scale;
}

// Direction vectors
DirectX::XMFLOAT3 Transform::GetForward() const {
    UpdateWorldMatrix();
    DirectX::XMFLOAT3 forward;
    DirectX::XMStoreFloat3(&forward, DirectX::XMVector3Normalize(DirectX::XMVectorNegate(m_worldMatrix.r[2])));
    return forward;
}

DirectX::XMFLOAT3 Transform::GetRight() const {
    UpdateWorldMatrix();
    DirectX::XMFLOAT3 right;
    DirectX::XMStoreFloat3(&right, DirectX::XMVector3Normalize(m_worldMatrix.r[0]));
    return right;
}

DirectX::XMFLOAT3 Transform::GetUp() const {
    UpdateWorldMatrix();
    DirectX::XMFLOAT3 up;
    DirectX::XMStoreFloat3(&up, DirectX::XMVector3Normalize(m_worldMatrix.r[1]));
    return up;
}

// Matrix access
DirectX::XMMATRIX Transform::GetLocalMatrix() const {
    UpdateLocalMatrix();
    return m_localMatrix;
}

DirectX::XMMATRIX Transform::GetWorldMatrix() const {
    UpdateWorldMatrix();
    return m_worldMatrix;
}

DirectX::XMMATRIX Transform::GetInverseWorldMatrix() const {
    return DirectX::XMMatrixInverse(nullptr, GetWorldMatrix());
}

// Transform operations
void Transform::Translate(const DirectX::XMFLOAT3& translation) {
    m_localPosition.x += translation.x;
    m_localPosition.y += translation.y;
    m_localPosition.z += translation.z;

    m_localMatrixDirty = true;
    MarkWorldMatrixDirty();
    MarkDirty();
}

void Transform::Translate(float x, float y, float z) {
    Translate(DirectX::XMFLOAT3(x, y, z));
}

void Transform::Rotate(const DirectX::XMFLOAT3& rotation) {
    m_localRotation.x += rotation.x;
    m_localRotation.y += rotation.y;
    m_localRotation.z += rotation.z;

    m_localMatrixDirty = true;
    MarkWorldMatrixDirty();
    MarkDirty();
}

void Transform::Rotate(float x, float y, float z) {
    Rotate(DirectX::XMFLOAT3(x, y, z));
}

void Transform::Scale(const DirectX::XMFLOAT3& scale) {
    m_localScale.x *= scale.x;
    m_localScale.y *= scale.y;
    m_localScale.z *= scale.z;

    m_localMatrixDirty = true;
    MarkWorldMatrixDirty();
    MarkDirty();
}

void Transform::Scale(float uniformScale) {
    Scale(DirectX::XMFLOAT3(uniformScale, uniformScale, uniformScale));
}

// Look at functionality
void Transform::LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up) {
    DirectX::XMFLOAT3 worldPos = GetWorldPosition();
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&worldPos);
    DirectX::XMVECTOR tar = DirectX::XMLoadFloat3(&target);
    DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);

    DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(tar, pos));
    LookDirection(DirectX::XMFLOAT3(
        DirectX::XMVectorGetX(direction),
        DirectX::XMVectorGetY(direction),
        DirectX::XMVectorGetZ(direction)
    ), up);
}

void Transform::LookDirection(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& up) {
    DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
    DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);

    // Create look matrix
    DirectX::XMMATRIX lookMatrix = DirectX::XMMatrixLookToLH(
        DirectX::XMVectorSet(0, 0, 0, 1), dir, upVec);

    // Convert to rotation
    DirectX::XMFLOAT3 rotation = MatrixToEuler(DirectX::XMMatrixInverse(nullptr, lookMatrix));
    SetWorldRotation(rotation);
}

// Transform points and vectors
DirectX::XMFLOAT3 Transform::TransformPoint(const DirectX::XMFLOAT3& point) const {
    DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&point);
    DirectX::XMVECTOR transformed = DirectX::XMVector3Transform(p, GetWorldMatrix());

    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, transformed);
    return result;
}

DirectX::XMFLOAT3 Transform::TransformDirection(const DirectX::XMFLOAT3& direction) const {
    DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
    DirectX::XMVECTOR transformed = DirectX::XMVector3TransformNormal(dir, GetWorldMatrix());

    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, transformed);
    return result;
}

DirectX::XMFLOAT3 Transform::InverseTransformPoint(const DirectX::XMFLOAT3& point) const {
    DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&point);
    DirectX::XMVECTOR transformed = DirectX::XMVector3Transform(p, GetInverseWorldMatrix());

    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, transformed);
    return result;
}

DirectX::XMFLOAT3 Transform::InverseTransformDirection(const DirectX::XMFLOAT3& direction) const {
    DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
    DirectX::XMVECTOR transformed = DirectX::XMVector3TransformNormal(dir, GetInverseWorldMatrix());

    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, transformed);
    return result;
}

// Hierarchy support
Transform* Transform::GetParent() const {
    if (m_entity && m_entity->GetParent()) {
        return m_entity->GetParent()->GetTransform();
    }
    return nullptr;
}

// Private methods
void Transform::UpdateLocalMatrix() const {
    if (m_localMatrixDirty) {
        // Create transformation matrices
        DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(
            m_localPosition.x, m_localPosition.y, m_localPosition.z);

        DirectX::XMMATRIX rotation = CreateRotationMatrix();

        DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(
            m_localScale.x, m_localScale.y, m_localScale.z);

        // Combine: Scale * Rotation * Translation
        m_localMatrix = DirectX::XMMatrixMultiply(
            DirectX::XMMatrixMultiply(scale, rotation), translation);

        m_localMatrixDirty = false;
    }
}

void Transform::UpdateWorldMatrix() const {
    if (m_worldMatrixDirty) {
        UpdateLocalMatrix();

        Transform* parent = GetParent();
        if (parent) {
            parent->UpdateWorldMatrix();
            m_worldMatrix = DirectX::XMMatrixMultiply(m_localMatrix, parent->m_worldMatrix);
        } else {
            m_worldMatrix = m_localMatrix;
        }

        m_worldMatrixDirty = false;
    }
}

DirectX::XMMATRIX Transform::CreateRotationMatrix() const {
    // Convert degrees to radians and create rotation matrix
    float radX = DirectX::XMConvertToRadians(m_localRotation.x);
    float radY = DirectX::XMConvertToRadians(m_localRotation.y);
    float radZ = DirectX::XMConvertToRadians(m_localRotation.z);

    return DirectX::XMMatrixRotationRollPitchYaw(radX, radY, radZ);
}

DirectX::XMFLOAT3 Transform::MatrixToEuler(const DirectX::XMMATRIX& matrix) const {
    // Extract Euler angles from rotation matrix (in degrees)
    DirectX::XMFLOAT4X4 mat;
    DirectX::XMStoreFloat4x4(&mat, matrix);

    float sy = sqrtf(mat._11 * mat._11 + mat._21 * mat._21);

    bool singular = sy < 1e-6f;

    float x, y, z;
    if (!singular) {
        x = atan2f(mat._32, mat._33);
        y = atan2f(-mat._31, sy);
        z = atan2f(mat._21, mat._11);
    } else {
        x = atan2f(-mat._23, mat._22);
        y = atan2f(-mat._31, sy);
        z = 0;
    }

    return DirectX::XMFLOAT3(
        DirectX::XMConvertToDegrees(x),
        DirectX::XMConvertToDegrees(y),
        DirectX::XMConvertToDegrees(z)
    );
}

void Transform::MarkWorldMatrixDirty() const {
    m_worldMatrixDirty = true;

    // Mark all children's world matrices dirty too
    if (m_entity) {
        for (Entity* child : m_entity->GetChildren()) {
            if (child && child->GetTransform()) {
                child->GetTransform()->MarkWorldMatrixDirty();
            }
        }
    }
}

} // namespace Scene
} // namespace GameEngine
