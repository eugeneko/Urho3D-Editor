#pragma once

#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Quaternion.h>

namespace Urho3D
{

class Scene;

/// Transformable interface.
class Transformable
{
public:
    /// Return scene.
    virtual Scene& GetScene() = 0;

    /// Return whether the transformation is empty.
    virtual bool IsEmpty() = 0;
    /// Return transformable position.
    virtual Vector3 GetPosition() = 0;
    /// Return transformable rotation.
    virtual Quaternion GetRotation() = 0;

    /// Start transformation.
    virtual void StartTransformation() = 0;
    /// Apply position change in world space.
    virtual void ApplyPositionChange(const Vector3& delta) = 0;
    /// Apply rotation change in world space.
    virtual void ApplyRotationChange(const Quaternion& delta) = 0;
    /// Apply scale change.
    virtual void ApplyScaleChange(const Vector3& delta) = 0;
    /// Snap scale values to grid.
    virtual void SnapScale(float step) = 0;

};

}

#include <Urho3D/Core/Object.h>

namespace Urho3D
{

class Selection;
class Node;

/// Transform of the node.
struct NodeTransform
{
    /// Define from node.
    void Define(const Node& node);
    /// Apply to node.
    void Apply(Node& node);

    /// Position.
    Vector3 position_;
    /// Rotation.
    Quaternion rotation_;
    /// Scale.
    Vector3 scale_;
};

class SelectionTransform : public Object, public Transformable
{
    URHO3D_OBJECT(SelectionTransform, Object);

public:
    /// Construct.
    SelectionTransform(Context* context) : Object(context) { }
    /// Set scene.
    void SetScene(Scene* scene) { scene_ = scene; }
    /// Set selection.
    void SetSelection(Selection* selection) { selection_ = selection; }

    /// \see Transformable::GetScene
    Scene& GetScene() override { return *scene_; }
    /// \see Transformable::GetScene
    bool IsEmpty() override;
    /// \see Transformable::GetScene
    Vector3 GetPosition() override;
    /// \see Transformable::GetScene
    Quaternion GetRotation() override;
    /// \see Transformable::GetScene
    void StartTransformation() override;
    /// \see Transformable::ApplyPositionChange
    void ApplyPositionChange(const Vector3& delta) override;
    /// \see Transformable::ApplyRotationChange
    void ApplyRotationChange(const Quaternion& delta) override;
    /// \see Transformable::ApplyScaleChange
    void ApplyScaleChange(const Vector3& delta) override;
    /// \see Transformable::ApplyScaleChange
    void SnapScale(float step) override;

private:
    Scene* scene_ = nullptr;
    Selection* selection_ = nullptr;
    Vector<Node*> nodes_;
    Vector<NodeTransform> initialTransforms_;
    //QVector<NodeTransform> oldTransforms_;

};


}
