#include "SceneActions.h"
#include "SceneDocument.h"
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
EditNodeTransformAction::EditNodeTransformAction(
    SceneDocument& document, const Urho3D::Node& node, const NodeTransform& oldTransform, QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , document_(document)
    , nodeID_(node.GetID())
    , undoTransform_(oldTransform)
{
    redoTransform_.Define(node);
}

void EditNodeTransformAction::undo()
{
    Urho3D::Scene& scene = document_.GetScene();
    if (Urho3D::Node* node = scene.GetNode(nodeID_))
    {
        undoTransform_.Apply(*node);
        emit document_.nodeTransformChanged(*node);
    }
}

void EditNodeTransformAction::redo()
{
    Urho3D::Scene& scene = document_.GetScene();
    if (Urho3D::Node* node = scene.GetNode(nodeID_))
    {
        redoTransform_.Apply(*node);
        emit document_.nodeTransformChanged(*node);
    }
}

//////////////////////////////////////////////////////////////////////////
CreateNodeAction::CreateNodeAction(SceneDocument& document,
    Urho3D::SharedPtr<Urho3D::XMLFile> desc, unsigned nodeId, unsigned parentId,
    QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , document_(document)
    , nodeId_(nodeId)
    , parentId_(parentId)
    , desc_(desc)
{
}

void CreateNodeAction::undo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();
    Node* parent = scene.GetNode(parentId_);
    Node* node = scene.GetNode(nodeId_);
    if (parent && node)
        parent->RemoveChild(node);
}

void CreateNodeAction::redo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* parent = scene.GetNode(parentId_);
    if (parent)
    {
        Node* node = parent->CreateChild("", nodeId_ < FIRST_LOCAL_ID ? REPLICATED : LOCAL, nodeId_);
        node->LoadXML(desc_->GetRoot());
        /// \todo Do we need focusing?
        //FocusNode(node);
    }
}

//////////////////////////////////////////////////////////////////////////
DeleteNodeAction::DeleteNodeAction(SceneDocument& document, Urho3D::Node& node, QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , document_(document)
    , nodeID(node.GetID())
    , parentID(node.GetParent()->GetID())
    , nodeData(new Urho3D::XMLFile(document.GetContext()))
    , index_(node.GetParent()->GetChildren().IndexOf(Urho3D::SharedPtr<Urho3D::Node>(&node)))
{
    Urho3D::XMLElement rootElem = nodeData->CreateRoot("node");
    node.SaveXML(rootElem);
}

void DeleteNodeAction::undo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* parent = scene.GetNode(parentID);
    if (parent)
    {
        Node* node = parent->CreateChild("", nodeID < FIRST_LOCAL_ID ? REPLICATED : LOCAL, nodeID);
        node->LoadXML(nodeData->GetRoot());
        parent->AddChild(node, index_);
        /// \todo Do we need focusing?
        //FocusNode(node);
    }
}

void DeleteNodeAction::redo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* parent = scene.GetNode(parentID);
    Node* node = scene.GetNode(nodeID);
    if (parent && node)
        parent->RemoveChild(node);
}

//////////////////////////////////////////////////////////////////////////
CreateComponentAction::CreateComponentAction(SceneDocument& document,
    Urho3D::SharedPtr<Urho3D::XMLFile> desc, unsigned componentId, unsigned nodeId,
    QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , document_(document)
    , componentId_(componentId)
    , nodeId_(nodeId)
    , desc_(desc)
{
}

void CreateComponentAction::undo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* node = scene.GetNode(nodeId_);
    Component* component = scene.GetComponent(componentId_);
    if (node && component)
        node->RemoveComponent(component);
}

void CreateComponentAction::redo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* node = scene.GetNode(nodeId_);
    if (node)
    {
        if (Component* component = node->CreateComponent(desc_->GetRoot().GetAttribute("type"),
            componentId_ < FIRST_LOCAL_ID ? REPLICATED : LOCAL, componentId_))
        {
            component->LoadXML(desc_->GetRoot());
            component->ApplyAttributes();
            /// \todo Do we need focusing?
            //FocusComponent(component);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
DeleteComponentAction::DeleteComponentAction(SceneDocument& document, Urho3D::Component& component, QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , document_(document)
    , componentID(component.GetID())
    , nodeID(component.GetNode()->GetID())
    , componentData(new Urho3D::XMLFile(document.GetContext()))
    , index_(component.GetNode()->GetComponents().IndexOf(Urho3D::SharedPtr<Urho3D::Component>(&component)))
{
    Urho3D::XMLElement rootElem = componentData->CreateRoot("component");
    component.SaveXML(rootElem);
}

void DeleteComponentAction::undo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* node = scene.GetNode(nodeID);
    if (node)
    {
        Component* component = node->CreateComponent(componentData->GetRoot().GetAttribute("type"),
            componentID < FIRST_LOCAL_ID ? REPLICATED : LOCAL, componentID);
        component->LoadXML(componentData->GetRoot());
        component->ApplyAttributes();
        node->ReorderComponent(component, index_);
        /// \todo Do we need focusing?
        //FocusComponent(component);
    }
}

void DeleteComponentAction::redo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* node = scene.GetNode(nodeID);
    Component* component = scene.GetComponent(componentID);
    if (node && component)
        node->RemoveComponent(component);
}

//////////////////////////////////////////////////////////////////////////
NodeHierarchyAction::NodeHierarchyAction(SceneDocument& document,
    unsigned nodeId, unsigned oldParentId, unsigned oldIndex, unsigned newParentId, unsigned newIndex,
    QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , document_(document)
    , nodeId(nodeId)
    , oldParentId(oldParentId)
    , oldIndex(oldIndex)
    , newParentId(newParentId)
    , newIndex(newIndex)
{
}

void NodeHierarchyAction::undo()
{
    MoveNode(oldParentId, oldIndex);
}

void NodeHierarchyAction::redo()
{
    MoveNode(newParentId, newIndex);
}

void NodeHierarchyAction::MoveNode(unsigned parentId, unsigned index) const
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* node = scene.GetNode(nodeId);
    Node* parent = scene.GetNode(parentId);
    if (node && parent)
    {
        // Re-parent if needed
        if (node->GetParent() != parent)
            node->SetParent(parent);

        // Re-order if needed
        if (parent->GetChild(index) != node)
        {
            // Store node to prevent destruction.
            SharedPtr<Node> nodeHolder(node);

            // Removal from scene zeroes the ID. Be prepared to restore it.
            const unsigned oldId = node->GetID();
            node->Remove();
            node->SetID(oldId);
            parent->AddChild(node, index);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
ComponentHierarchyAction::ComponentHierarchyAction(SceneDocument& document,
    unsigned nodeId, unsigned componentId, unsigned oldIndex, unsigned newIndex, QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , document_(document)
    , nodeId(nodeId)
    , componentId(componentId)
    , oldIndex(oldIndex)
    , newIndex(newIndex)
{
}

void ComponentHierarchyAction::undo()
{
    MoveComponent(oldIndex);
}

void ComponentHierarchyAction::redo()
{
    MoveComponent(newIndex);
}

void ComponentHierarchyAction::MoveComponent(unsigned index)
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* node = scene.GetNode(nodeId);
    Component* component = scene.GetComponent(componentId);
    if (node && component)
    {
        node->ReorderComponent(component, index);
        emit document_.componentReordered(*component, index);
    }
}

}
