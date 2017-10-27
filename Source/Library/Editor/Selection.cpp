#include "Selection.h"
#include "EditorEvents.h"
#include <Urho3D/Graphics/Drawable.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>

// #include "SceneDocument.h"
// #include "SceneActions.h"
// #include "SceneOverlay.h"
// #include "SceneViewportManager.h"
// #include "../Core/QtUrhoHelpers.h"
// #include "../Configuration.h"
// #include "../Core/Core.h"
// #include "../Widgets/Urho3DWidget.h"
// #include <Urho3D/Core/CoreEvents.h>
// #include <Urho3D/Graphics/DebugRenderer.h>
// #include <Urho3D/Graphics/Octree.h>
// #include <Urho3D/IO/File.h>
// #include <Urho3D/Input/Input.h>
// #include <Urho3D/Scene/SceneEvents.h>
// #include <QFileInfo>
// #include <QKeyEvent>

// #TODO Extract this code
// #include "DebugRenderer.h"
// #include "Gizmo.h"
// #include "ObjectPicker.h"
// #include <QMessageBox>

namespace Urho3D
{

void Selection::ClearSelection()
{
    selectedObjectsVector_.Clear();
    selectedObjectsSet_.Clear();
    UpdateChangedSelection();
}

void Selection::SetSelection(const ObjectVector& objects)
{
    selectedObjectsVector_.Clear();
    selectedObjectsSet_.Clear();
    for (Object* object : objects)
    {
        if (object)
        {
            WeakPtr<Object> weakObject(object);
            selectedObjectsVector_.Push(weakObject);
            selectedObjectsSet_.Insert(weakObject);
        }
    }

    UpdateChangedSelection();
}

void Selection::SelectObject(Object* object, SelectionAction action, bool clearSelection)
{
    if (clearSelection)
    {
        selectedObjectsVector_.Clear();
        selectedObjectsSet_.Clear();
    }

    WeakPtr<Object> weakObject(object);
    const bool wasSelected = selectedObjectsSet_.Contains(weakObject);
    if (action != SelectionAction::Select && wasSelected)
    {
        selectedObjectsSet_.Erase(weakObject);
        selectedObjectsVector_.Remove(weakObject);
    }
    if (action != SelectionAction::Deselect && !wasSelected)
    {
        selectedObjectsSet_.Insert(weakObject);
        selectedObjectsVector_.Push(weakObject);
    }

    UpdateChangedSelection();
}

void Selection::SetHoveredObject(Object* object)
{
    hoveredObject_ = object;
}

Vector3 Selection::GetSelectedCenter()
{
    using namespace Urho3D;

    const unsigned count = selectedNodes_.Size() + selectedComponents_.Size();
    Vector3 centerPoint;

    // Accumulate nodes
    for (Node* node : selectedNodes_)
        centerPoint += node->GetWorldPosition();

    // Accumulate components
    for (Component* component : selectedComponents_)
    {
        Drawable* drawable = dynamic_cast<Drawable*>(component);
        if (drawable)
            centerPoint += drawable->GetNode()->LocalToWorld(drawable->GetBoundingBox().Center());
        else
            centerPoint += component->GetNode()->GetWorldPosition();
    }

    if (count > 0)
        lastSelectedCenter_ = centerPoint / static_cast<float>(count);
    return lastSelectedCenter_;
}

Node* Selection::GetHoveredNode() const
{
    return dynamic_cast<Node*>(hoveredObject_);
}

Component* Selection::GetHoveredComponent() const
{
    return dynamic_cast<Component*>(hoveredObject_);
}

void Selection::UpdateChangedSelection()
{
    selectedNodes_.Clear();
    selectedComponents_.Clear();
    selectedNodesAndComponents_.Clear();
    for (Object* object : selectedObjectsVector_)
    {
        if (Node* node = dynamic_cast<Node*>(object))
        {
            WeakPtr<Node> weakNode(node);
            selectedNodes_.Push(weakNode);
            selectedNodesAndComponents_.Push(weakNode);
        }
        if (Component* component = dynamic_cast<Component*>(object))
        {
            WeakPtr<Component> weakComponent(component);
            WeakPtr<Node> weakNode(component->GetNode());
            selectedComponents_.Push(weakComponent);
            selectedNodesAndComponents_.Push(weakNode);
        }
    }

    if (onSelectionChanged_)
        onSelectionChanged_();
}

}
