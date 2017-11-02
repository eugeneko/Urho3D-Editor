#pragma once

#include "UndoStack.h"
#include "Selection.h" // #TODO Hide it
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
    /// End transformation.
    virtual void EndTransformation() = 0;

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
    void Apply(Node& node) const;

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
    /// Set undo stack.
    void SetUndoStack(const SharedPtr<UndoStack>& undoStack) { undoStack_ = undoStack; }
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
    /// \see Transformable::SnapScale
    void SnapScale(float step) override;
    /// \see Transformable::EndTransformation
    void EndTransformation() override;

private:
    SharedPtr<UndoStack> undoStack_;
    WeakPtr<Scene> scene_;
    WeakPtr<Selection> selection_;
    Vector<WeakPtr<Node>> nodes_;
    Vector<NodeTransform> initialTransforms_;
};

// #TODO Move it
/// Node transform edited.
class SelectionTransformChanged : public UndoCommand
{
public:
    /// Construct.
    SelectionTransformChanged(Context* context, Scene* scene, unsigned nodeId,
        const NodeTransform& oldTransform, const NodeTransform& newTransform);

    void Undo() const override;
    void Redo() const override;
    String GetTitle() override { return "Node Transform"; }

private:
    WeakPtr<Scene> scene_;
    unsigned nodeId_;
    NodeTransform oldTransform_;
    NodeTransform newTransform_;
};

}
