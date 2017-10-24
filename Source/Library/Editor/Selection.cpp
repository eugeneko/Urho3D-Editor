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
    selectedObjects_.Clear();
    UpdateChangedSelection();
}

void Selection::SetSelection(const ObjectSet& objects)
{
    selectedObjects_ = objects;
    UpdateChangedSelection();
}

void Selection::SelectObject(Object* object, SelectionAction action, bool clearSelection)
{
    if (clearSelection)
        selectedObjects_.Clear();

    const bool wasSelected = selectedObjects_.Erase(object);
    if (!wasSelected && action != SelectionAction::Deselect)
        selectedObjects_.Insert(object);

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
    for (Object* object : selectedObjects_)
    {
        if (Node* node = dynamic_cast<Node*>(object))
        {
            selectedNodes_.Insert(node);
            selectedNodesAndComponents_.Insert(node);
        }
        if (Component* component = dynamic_cast<Component*>(object))
        {
            selectedComponents_.Insert(component);
            selectedNodesAndComponents_.Insert(component->GetNode());
        }
    }

    if (onSelectionChanged_)
        onSelectionChanged_();
}

}
