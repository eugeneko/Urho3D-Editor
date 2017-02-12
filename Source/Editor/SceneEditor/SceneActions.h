#pragma once

#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Quaternion.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Resource/XMLFile.h>
#include <QUndoCommand>
#include <QVector>

namespace Urho3D
{

class Scene;
class Node;
class Component;
class Serializable;

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
    unsigned nodeId_;
    /// Old transform.
    NodeTransform undoTransform_;
    /// New transform.
    NodeTransform redoTransform_;

};

/// Node created.
class CreateNodeAction : public QUndoCommand
{
public:
    /// Construct.
    CreateNodeAction(SceneDocument& document,
        Urho3D::SharedPtr<Urho3D::XMLFile> desc, unsigned nodeId, unsigned parentId,
        QUndoCommand* parent = nullptr);

    /// @see QUndoCommand::undo
    virtual void undo() override;

    /// @see QUndoCommand::redo
    virtual void redo() override;

private:
    /// Document.
    SceneDocument& document_;
    /// Node ID.
    uint nodeId_;
    /// Parent node ID.
    uint parentId_;
    /// Node data.
    Urho3D::SharedPtr<Urho3D::XMLFile> desc_;

};

/// Node deleted.
class DeleteNodeAction : public QUndoCommand
{
public:
    /// Construct.
    DeleteNodeAction(SceneDocument& document, Urho3D::Node& node, QUndoCommand* parent = nullptr);

    /// @see QUndoCommand::undo
    virtual void undo() override;

    /// @see QUndoCommand::redo
    virtual void redo() override;

private:
    /// Document.
    SceneDocument& document_;
    /// Node ID.
    uint nodeId_;
    /// Parent node ID.
    uint parentId_;
    /// Node data.
    Urho3D::SharedPtr<Urho3D::XMLFile> nodeData_;
    /// Node index in parent.
    unsigned index_;

};

/// Component created.
class CreateComponentAction : public QUndoCommand
{
public:
    /// Construct.
    CreateComponentAction(SceneDocument& document,
        Urho3D::SharedPtr<Urho3D::XMLFile> desc, unsigned componentId, unsigned nodeId,
        QUndoCommand* parent = nullptr);

    /// @see QUndoCommand::undo
    virtual void undo() override;

    /// @see QUndoCommand::redo
    virtual void redo() override;

private:
    /// Document.
    SceneDocument& document_;
    /// Component ID.
    uint componentId_;
    /// Parent node ID.
    uint nodeId_;
    /// Component data.
    Urho3D::SharedPtr<Urho3D::XMLFile> desc_;

};

/// Component deleted.
class DeleteComponentAction : public QUndoCommand
{
public:
    /// Construct.
    DeleteComponentAction(SceneDocument& document, Urho3D::Component& component, QUndoCommand* parent = nullptr);

    /// @see QUndoCommand::undo
    virtual void undo() override;

    /// @see QUndoCommand::redo
    virtual void redo() override;

private:
    /// Document.
    SceneDocument& document_;
    /// Component ID.
    uint componentId_;
    /// Parent node ID.
    uint nodeId_;
    /// Component data.
    Urho3D::SharedPtr<Urho3D::XMLFile> componentData_;
    /// Node index in parent.
    unsigned index_;

};

/// Node re-parented or re-ordered.
class NodeHierarchyAction : public QUndoCommand
{
public:
    /// Construct.
    NodeHierarchyAction(SceneDocument& document,
        unsigned nodeId, unsigned oldParentId, unsigned oldIndex,
        unsigned newParentId, unsigned newIndex,
        QUndoCommand* parent = nullptr);

    /// @see QUndoCommand::undo
    virtual void undo() override;

    /// @see QUndoCommand::redo
    virtual void redo() override;

private:
    /// Move node to specified parent and index.
    void MoveNode(unsigned parentId, unsigned index) const;

private:
    /// Document.
    SceneDocument& document_;
    /// Node ID.
    unsigned nodeId_;
    /// Old parent node ID.
    unsigned oldParentId_;
    /// Old index of node.
    unsigned oldIndex_;
    /// New parent node ID.
    unsigned newParentId_;
    /// New index of node.
    unsigned newIndex_;
};

/// Component re-ordered.
class ComponentHierarchyAction : public QUndoCommand
{
public:
    /// Construct.
    ComponentHierarchyAction(SceneDocument& document,
        unsigned nodeId, unsigned componentId, unsigned oldIndex, unsigned newIndex,
        QUndoCommand* parent = nullptr);

    /// @see QUndoCommand::undo
    virtual void undo() override;

    /// @see QUndoCommand::redo
    virtual void redo() override;

private:
    /// Move component.
    void MoveComponent(unsigned index);

private:
    /// Document.
    SceneDocument& document_;
    /// Node ID.
    unsigned nodeId_;
    /// Component ID.
    unsigned componentId_;
    /// Old index.
    unsigned oldIndex_;
    /// New index.
    unsigned newIndex_;
};

/// Serializable type.
enum class SerializableType
{
    /// Node.
    Node,
    /// Component.
    Component
};

/// Get serializable type.
SerializableType GetSerializableType(Urho3D::Serializable& serializable);

/// Serializable attribute modified.
struct EditSerializableAttributeAction
{
    /// Serializable ID.
    unsigned serializableId_;
    /// Old value.
    Urho3D::Variant oldValue_;
    /// New value.
    Urho3D::Variant newValue_;
};

/// Multiple serializables attribute modified.
class EditMultipleSerializableAttributeAction : public QUndoCommand
{
public:
    /// Construct.
    EditMultipleSerializableAttributeAction(SceneDocument& document,
        SerializableType type, unsigned generation, unsigned attribute,
        const QVector<EditSerializableAttributeAction>& actions);

    /// @see QUndoCommand::id
    virtual int id() const override { return 0; }
    /// @see QUndoCommand::mergeWith
    virtual bool mergeWith(const QUndoCommand* other) override;
    /// @see QUndoCommand::undo
    virtual void undo() override;
    /// @see QUndoCommand::redo
    virtual void redo() override;

private:
    /// Get serializable by ID.
    Urho3D::Serializable* GetSerializable(unsigned id);
    /// Set attribute value.
    void SetAttribute(unsigned serializableId, const Urho3D::Variant& value);

private:
    /// Document.
    SceneDocument& document_;
    /// Serializable type.
    SerializableType type_;
    /// Generation.
    unsigned generation_;
    /// Attribute.
    unsigned attribute_;
    /// Actions.
    QVector<EditSerializableAttributeAction> actions_;

};

}
