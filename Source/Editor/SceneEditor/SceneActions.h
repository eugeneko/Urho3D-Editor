#pragma once

#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Quaternion.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Resource/XMLFile.h>
#include <QUndoCommand>

namespace Urho3D
{

class Scene;
class Node;
class Component;

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
    uint nodeID;
    /// Parent node ID.
    uint parentID;
    /// Node data.
    Urho3D::SharedPtr<Urho3D::XMLFile> nodeData;
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
    uint componentID;
    /// Parent node ID.
    uint nodeID;
    /// Component data.
    Urho3D::SharedPtr<Urho3D::XMLFile> componentData;
    /// Node index in parent.
    unsigned index_;

};

}
