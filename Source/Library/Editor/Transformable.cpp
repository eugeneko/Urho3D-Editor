#include "Transformable.h"

#include "Selection.h"
#include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

namespace
{

/// Snap vector to grid.
/// \todo Move to math
Vector3 SnapVector(Vector3 vector, float step)
{
    vector.x_ = Round(vector.x_ / step) * step;
    vector.y_ = Round(vector.y_ / step) * step;
    vector.z_ = Round(vector.z_ / step) * step;
    return vector;
}

}

void NodeTransform::Define(const Node& node)
{
    position_ = node.GetPosition();
    rotation_ = node.GetRotation();
    scale_ = node.GetScale();
}

void NodeTransform::Apply(Node& node)
{
    node.SetTransform(position_, rotation_, scale_);
}

//////////////////////////////////////////////////////////////////////////
bool SelectionTransform::IsEmpty()
{
    const Selection::NodeSet& nodes = selection_->GetSelectedNodesAndComponents();
    const bool sceneSelected = nodes.Contains(scene_);
    return sceneSelected || nodes.Empty();
}

Vector3 SelectionTransform::GetPosition()
{
    const Selection::NodeSet& nodes = selection_->GetSelectedNodesAndComponents();

    Vector3 center;
    for (Node* node : nodes)
        center += node->GetWorldPosition();
    center /= static_cast<float>(nodes.Size());

    return center;
}

Quaternion SelectionTransform::GetRotation()
{
    const Selection::NodeSet& nodes = selection_->GetSelectedNodesAndComponents();

    return nodes.Size() == 1
        ? (**nodes.Begin()).GetWorldRotation()
        : Quaternion::IDENTITY;
}

void SelectionTransform::StartTransformation()
{
    const Selection::NodeSet& selection = selection_->GetSelectedNodesAndComponents();

    nodes_.Clear();
    for (Node* node : selection)
        nodes_.Push(node);

    initialTransforms_.Resize(selection.Size());
    for (unsigned i = 0; i < selection.Size(); ++i)
        initialTransforms_[i].Define(*nodes_[i]);
}

void SelectionTransform::ApplyPositionChange(const Vector3& delta)
{
    for (Node* node : nodes_)
        node->SetWorldPosition(node->GetWorldPosition() + delta);
}

void SelectionTransform::ApplyRotationChange(const Quaternion& delta)
{
    const Vector3 origin = GetPosition();
    for (Node* node : nodes_)
    {
        const Vector3 offset = node->GetWorldPosition() - origin;
        node->SetWorldRotation(delta * node->GetWorldRotation());
        node->SetWorldPosition(origin + delta * offset);
    }
}

void SelectionTransform::ApplyScaleChange(const Vector3& delta)
{
    for (Node* node : nodes_)
        node->SetScale(node->GetScale() + delta);
}

void SelectionTransform::SnapScale(float step)
{
    for (Node* node : nodes_)
        node->SetScale(SnapVector(node->GetScale(), step));
}

}
