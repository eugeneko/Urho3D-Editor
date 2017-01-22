#pragma once

#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Quaternion.h>
#include <QUndoCommand>

namespace Urho3D
{

class Scene;
class Node;

}

namespace Urho3DEditor
{

class SceneDocument;

/// Transform of the node.
struct NodeTransform
{
    /// Define from node.
    void Define(const Urho3D::Node& node);
    /// Apply to node.
    void Apply(Urho3D::Node& node);

    /// Position.
    Urho3D::Vector3 position_;
    /// Rotation.
    Urho3D::Quaternion rotation_;
    /// Scale.
    Urho3D::Vector3 scale_;
};

/// Node transform edited.
class EditNodeTransformAction : public QUndoCommand
{
public:
    /// Construct.
    EditNodeTransformAction(SceneDocument& document,
        const Urho3D::Node& node, const NodeTransform& oldTransform, QUndoCommand* parent = nullptr);

    /// @see QUndoCommand::undo
    virtual void undo() override;
    /// @see QUndoCommand::redo
    virtual void redo() override;

private:
    /// Document.
    SceneDocument& document_;
    /// Node ID.
    unsigned nodeID_;
    /// Old transform.
    NodeTransform undoTransform_;
    /// New transform.
    NodeTransform redoTransform_;

};

}
