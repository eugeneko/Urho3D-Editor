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
    return selection_->IsSelected(scene_) || selection_->GetNodesAndComponents().Empty();
}

Vector3 SelectionTransform::GetPosition()
{
    const Selection::NodeVector& nodes = selection_->GetNodesAndComponents();

    Vector3 center;
    for (Node* node : nodes)
        center += node->GetWorldPosition();
    center /= static_cast<float>(nodes.Size());

    return center;
}

Quaternion SelectionTransform::GetRotation()
{
    const Selection::NodeVector& nodes = selection_->GetNodesAndComponents();

    return nodes.Size() == 1
        ? (**nodes.Begin()).GetWorldRotation()
        : Quaternion::IDENTITY;
}

void SelectionTransform::StartTransformation()
{
    const Selection::NodeVector& selection = selection_->GetNodesAndComponents();

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
