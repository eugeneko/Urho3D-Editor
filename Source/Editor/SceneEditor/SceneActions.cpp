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
    , nodeId_(node.GetID())
    , undoTransform_(oldTransform)
{
    redoTransform_.Define(node);
}

void EditNodeTransformAction::undo()
{
    Urho3D::Scene& scene = document_.GetScene();
    if (Urho3D::Node* node = scene.GetNode(nodeId_))
    {
        undoTransform_.Apply(*node);
        emit document_.nodeTransformChanged(*node);
    }
}

void EditNodeTransformAction::redo()
{
    Urho3D::Scene& scene = document_.GetScene();
    if (Urho3D::Node* node = scene.GetNode(nodeId_))
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
    , nodeId_(node.GetID())
    , parentId_(node.GetParent()->GetID())
    , nodeData_(new Urho3D::XMLFile(document.GetContext()))
    , index_(node.GetParent()->GetChildren().IndexOf(Urho3D::SharedPtr<Urho3D::Node>(&node)))
{
    Urho3D::XMLElement rootElem = nodeData_->CreateRoot("node");
    node.SaveXML(rootElem);
}

void DeleteNodeAction::undo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* parent = scene.GetNode(parentId_);
    if (parent)
    {
        Node* node = parent->CreateChild("", nodeId_ < FIRST_LOCAL_ID ? REPLICATED : LOCAL, nodeId_);
        node->LoadXML(nodeData_->GetRoot());
        parent->AddChild(node, index_);
        /// \todo Do we need focusing?
        //FocusNode(node);
    }
}

void DeleteNodeAction::redo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* parent = scene.GetNode(parentId_);
    Node* node = scene.GetNode(nodeId_);
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
    , componentId_(component.GetID())
    , nodeId_(component.GetNode()->GetID())
    , componentData_(new Urho3D::XMLFile(document.GetContext()))
    , index_(component.GetNode()->GetComponents().IndexOf(Urho3D::SharedPtr<Urho3D::Component>(&component)))
{
    Urho3D::XMLElement rootElem = componentData_->CreateRoot("component");
    component.SaveXML(rootElem);
}

void DeleteComponentAction::undo()
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* node = scene.GetNode(nodeId_);
    if (node)
    {
        Component* component = node->CreateComponent(componentData_->GetRoot().GetAttribute("type"),
            componentId_ < FIRST_LOCAL_ID ? REPLICATED : LOCAL, componentId_);
        component->LoadXML(componentData_->GetRoot());
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

    Node* node = scene.GetNode(nodeId_);
    Component* component = scene.GetComponent(componentId_);
    if (node && component)
        node->RemoveComponent(component);
}

//////////////////////////////////////////////////////////////////////////
NodeHierarchyAction::NodeHierarchyAction(SceneDocument& document,
    unsigned nodeId, unsigned oldParentId, unsigned oldIndex, unsigned newParentId, unsigned newIndex,
    QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , document_(document)
    , nodeId_(nodeId)
    , oldParentId_(oldParentId)
    , oldIndex_(oldIndex)
    , newParentId_(newParentId)
    , newIndex_(newIndex)
{
}

void NodeHierarchyAction::undo()
{
    MoveNode(oldParentId_, oldIndex_);
}

void NodeHierarchyAction::redo()
{
    MoveNode(newParentId_, newIndex_);
}

void NodeHierarchyAction::MoveNode(unsigned parentId, unsigned index) const
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* node = scene.GetNode(nodeId_);
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
    , nodeId_(nodeId)
    , componentId_(componentId)
    , oldIndex_(oldIndex)
    , newIndex_(newIndex)
{
}

void ComponentHierarchyAction::undo()
{
    MoveComponent(oldIndex_);
}

void ComponentHierarchyAction::redo()
{
    MoveComponent(newIndex_);
}

void ComponentHierarchyAction::MoveComponent(unsigned index)
{
    using namespace Urho3D;
    Scene& scene = document_.GetScene();

    Node* node = scene.GetNode(nodeId_);
    Component* component = scene.GetComponent(componentId_);
    if (node && component)
    {
        node->ReorderComponent(component, index);
        emit document_.componentReordered(*component, index);
    }
}

//////////////////////////////////////////////////////////////////////////
SerializableType GetSerializableType(Urho3D::Serializable& serializable)
{
    if (dynamic_cast<Urho3D::Node*>(&serializable))
        return SerializableType::Node;
    else if (dynamic_cast<Urho3D::Component*>(&serializable))
        return SerializableType::Component;

    assert(0);
    return SerializableType::Node;
}

//////////////////////////////////////////////////////////////////////////
EditMultipleSerializableAttributeAction::EditMultipleSerializableAttributeAction(SceneDocument& document,
    SerializableType type, unsigned generation, unsigned attribute, const QVector<EditSerializableAttributeAction>& actions)
    : document_(document)
    , type_(type)
    , generation_(generation)
    , attribute_(attribute)
    , actions_(actions)
{
}

bool EditMultipleSerializableAttributeAction::mergeWith(const QUndoCommand* other)
{
    // Check basic compatibility
    const EditMultipleSerializableAttributeAction* next = dynamic_cast<const EditMultipleSerializableAttributeAction*>(other);
    if (!next || next->generation_ != generation_ || next->attribute_ != attribute_
        || next->actions_.size() != actions_.size() || next->type_ != type_)
        return false;

    // Check serializables matching
    for (int i = 0; i < actions_.size(); ++i)
        if (next->actions_[i].serializableId_ != actions_[i].serializableId_)
            return false;

    // Merge
    for (int i = 0; i < actions_.size(); ++i)
    {
        assert(actions_[i].newValue_ == next->actions_[i].oldValue_);
        actions_[i].newValue_ = next->actions_[i].newValue_;
    }
    return true;
}

void EditMultipleSerializableAttributeAction::undo()
{
    for (int i = actions_.size() - 1; i >= 0; --i)
        SetAttribute(actions_[i].serializableId_, actions_[i].oldValue_);
}

void EditMultipleSerializableAttributeAction::redo()
{
    for (int i = 0; i < actions_.size(); ++i)
        SetAttribute(actions_[i].serializableId_, actions_[i].newValue_);
}

Urho3D::Serializable* EditMultipleSerializableAttributeAction::GetSerializable(unsigned id)
{
    Urho3D::Scene& scene = document_.GetScene();
    switch (type_)
    {
    case SerializableType::Node:
        return scene.GetNode(id);
    case SerializableType::Component:
        return scene.GetComponent(id);
    }
    return nullptr;
}

void EditMultipleSerializableAttributeAction::SetAttribute(unsigned serializableId, const Urho3D::Variant& value)
{
    if (Urho3D::Serializable* serializable = GetSerializable(serializableId))
    {
        assert(attribute_ < serializable->GetNumAttributes());
        serializable->SetAttribute(attribute_, value);
        emit document_.attributeChanged(*serializable, attribute_);
    }
}

}
