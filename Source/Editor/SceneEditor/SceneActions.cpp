#include "SceneActions.h"
#include "SceneEditor.h" // #TODO Split it
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3DEditor
{

void NodeTransform::Define(const Urho3D::Node& node)
{
    position_ = node.GetPosition();
    rotation_ = node.GetRotation();
    scale_ = node.GetScale();
}

void NodeTransform::Apply(Urho3D::Node& node)
{
    node.SetTransform(position_, rotation_, scale_);
}

//////////////////////////////////////////////////////////////////////////
EditNodeTransformAction::EditNodeTransformAction(SceneDocument& document, const Urho3D::Node& node, const NodeTransform& oldTransform)
    : document_(document)
    , nodeID_(node.GetID())
    , undoTransform_(oldTransform)
{
    redoTransform_.Define(node);
}

void EditNodeTransformAction::Undo()
{
    Urho3D::Scene& scene = document_.GetScene();
    if (Urho3D::Node* node = scene.GetNode(nodeID_))
    {
        undoTransform_.Apply(*node);
        emit document_.nodeTransformChanged(*node);
    }
}

void EditNodeTransformAction::Redo()
{
    Urho3D::Scene& scene = document_.GetScene();
    if (Urho3D::Node* node = scene.GetNode(nodeID_))
    {
        redoTransform_.Apply(*node);
        emit document_.nodeTransformChanged(*node);
    }
}

}
