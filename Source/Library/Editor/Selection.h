#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/HashSet.h>
// #include "SceneOverlay.h"
// #include "../Core/Document.h"
// #include <QAction>
// #include <QSet>
// #include <QUndoStack>
// #include <Urho3D/Graphics/Camera.h>
// #include <Urho3D/Graphics/Viewport.h>
// #include <Urho3D/Scene/Node.h>
// #include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

class Input;
class Node;
class Component;

/// Selection action.
enum class SelectionAction
{
    /// Select object.
    Select,
    /// Deselect object.
    Deselect,
    /// Flip selection.
    Toggle
};

/// Selection.
class Selection : public Object
{
    URHO3D_OBJECT(Selection, Object);

public:
    /// Vector of objects.
    using ObjectVector = Vector<WeakPtr<Object>>;
    /// Set of objects.
    using ObjectSet = HashSet<WeakPtr<Object>>;

    /// Vector of nodes.
    using NodeVector = Vector<WeakPtr<Node>>;
    /// Vector of components.
    using ComponentVector = Vector<WeakPtr<Component>>;
    /// Construct.
    Selection(Context* context) : Object(context) { }

    /// Clear selection.
    void ClearSelection();
    /// Select objects.
    void SetSelection(const ObjectVector& objects);
    /// Select object.
    void SelectObject(Object* object, SelectionAction action, bool clearSelection);
    /// Set hovered object.
    void SetHoveredObject(Object* object);

    /// Return whether the object is selected.
    bool IsSelected(Object* object) const { return selectedObjectsSet_.Contains(WeakPtr<Object>(object)); }
    /// Get vector of selected objects.
    const ObjectVector& GetObjects() const { return selectedObjectsVector_; }
    /// Get set of selected objects.
    const ObjectSet& GetObjectsSet() const { return selectedObjectsSet_; }

    /// Get selected nodes.
    const NodeVector& GetNodes() const { return selectedNodes_; }
    /// Get selected components.
    const ComponentVector& GetComponents() const { return selectedComponents_; }
    /// Get selected nodes and components.
    const NodeVector& GetNodesAndComponents() const { return selectedNodesAndComponents_; }
    /// Get center point of selected nodes.
    Vector3 GetSelectedCenter();
    /// Get hovered object.
    Object* GetHoveredObject() const { return hoveredObject_; }
    /// Get hovered node.
    Node* GetHoveredNode() const;
    /// Get hovered component.
    Component* GetHoveredComponent() const;

public:
    /// Called when selection is changed.
    std::function<void()> onSelectionChanged_;

private:
    /// Gather secondary selection lists.
    void UpdateChangedSelection();

private:
    /// Vector of selected objects.
    ObjectVector selectedObjectsVector_;
    /// Set of selected objects.
    ObjectSet selectedObjectsSet_;

    /// Selected nodes.
    NodeVector selectedNodes_;
    /// Selected components.
    ComponentVector selectedComponents_;
    /// Selected nodes and components.
    NodeVector selectedNodesAndComponents_;
    /// Hovered object.
    Object* hoveredObject_ = nullptr;
    /// Last center of selected nodes and components.
    /// \todo Is it needed?
    Vector3 lastSelectedCenter_;

};

}

