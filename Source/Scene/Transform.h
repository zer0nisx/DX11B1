#pragma once

#include "Component.h"
#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"
#include <DirectXMath.h>

namespace GameEngine {
namespace Scene {

class Transform : public Component {
public:
    COMPONENT_TYPE(Transform)

    Transform();
    virtual ~Transform() = default;

    // Position
    const DirectX::XMFLOAT3& GetLocalPosition() const { return m_localPosition; }
    void SetLocalPosition(const DirectX::XMFLOAT3& position);
    void SetLocalPosition(float x, float y, float z);

    DirectX::XMFLOAT3 GetWorldPosition() const;
    void SetWorldPosition(const DirectX::XMFLOAT3& position);
    void SetWorldPosition(float x, float y, float z);

    // Rotation (stored as Euler angles in degrees)
    const DirectX::XMFLOAT3& GetLocalRotation() const { return m_localRotation; }
    void SetLocalRotation(const DirectX::XMFLOAT3& rotation);
    void SetLocalRotation(float x, float y, float z);

    DirectX::XMFLOAT3 GetWorldRotation() const;
    void SetWorldRotation(const DirectX::XMFLOAT3& rotation);

    // Scale
    const DirectX::XMFLOAT3& GetLocalScale() const { return m_localScale; }
    void SetLocalScale(const DirectX::XMFLOAT3& scale);
    void SetLocalScale(float x, float y, float z);
    void SetLocalScale(float uniformScale);

    DirectX::XMFLOAT3 GetWorldScale() const;

    // Direction vectors
    DirectX::XMFLOAT3 GetForward() const;
    DirectX::XMFLOAT3 GetRight() const;
    DirectX::XMFLOAT3 GetUp() const;

    // Matrix access
    DirectX::XMMATRIX GetLocalMatrix() const;
    DirectX::XMMATRIX GetWorldMatrix() const;
    DirectX::XMMATRIX GetInverseWorldMatrix() const;

    // Transform operations
    void Translate(const DirectX::XMFLOAT3& translation);
    void Translate(float x, float y, float z);
    void Rotate(const DirectX::XMFLOAT3& rotation);
    void Rotate(float x, float y, float z);
    void Scale(const DirectX::XMFLOAT3& scale);
    void Scale(float uniformScale);

    // Look at functionality
    void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up = DirectX::XMFLOAT3(0, 1, 0));
    void LookDirection(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& up = DirectX::XMFLOAT3(0, 1, 0));

    // Transform points and vectors
    DirectX::XMFLOAT3 TransformPoint(const DirectX::XMFLOAT3& point) const;
    DirectX::XMFLOAT3 TransformDirection(const DirectX::XMFLOAT3& direction) const;
    DirectX::XMFLOAT3 InverseTransformPoint(const DirectX::XMFLOAT3& point) const;
    DirectX::XMFLOAT3 InverseTransformDirection(const DirectX::XMFLOAT3& direction) const;

    // Hierarchy support
    Transform* GetParent() const;
    bool HasParent() const { return GetParent() != nullptr; }

    // Dirty flag for optimization
    bool IsDirty() const { return m_isDirty; }
    void MarkDirty() { m_isDirty = true; }
    void MarkClean() { m_isDirty = false; }

    // Lifecycle
    virtual void OnStart() override;

private:
    // Local transform data
    DirectX::XMFLOAT3 m_localPosition;
    DirectX::XMFLOAT3 m_localRotation; // Euler angles in degrees
    DirectX::XMFLOAT3 m_localScale;

    // Cached matrices
    mutable DirectX::XMMATRIX m_localMatrix;
    mutable DirectX::XMMATRIX m_worldMatrix;
    mutable bool m_localMatrixDirty;
    mutable bool m_worldMatrixDirty;
    bool m_isDirty;

    // Helper methods
    void UpdateLocalMatrix() const;
    void UpdateWorldMatrix() const;
    DirectX::XMMATRIX CreateRotationMatrix() const;
    DirectX::XMFLOAT3 MatrixToEuler(const DirectX::XMMATRIX& matrix) const;

    // Mark world matrix dirty for this transform and all children
    void MarkWorldMatrixDirty() const;
};

} // namespace Scene
} // namespace GameEngine
