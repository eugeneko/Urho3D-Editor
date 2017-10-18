#include "ObjectSelector.h"
#include "Selection.h"
#include "../AbstractUI/AbstractInput.h"
#include "../AbstractUI/KeyBinding.h"
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>

namespace Urho3D
{

void ObjectSelector::SetScene(Scene* scene)
{
    scene_ = scene;
}

void ObjectSelector::SetSelection(Selection* selection)
{
    selection_ = selection;
}

void ObjectSelector::SetControls(const Controls& controls)
{
    controls_ = controls;
}

void ObjectSelector::AddSelectionTransferring(const String& sourceType, const String& destType)
{
    selectionTransferringRoutines_[sourceType] = destType;
}

void ObjectSelector::PostRenderUpdate(AbstractInput& input, AbstractEditorContext& editorContext)
{
    if (!input.IsUIHovered())
        PerformRaycast(input, editorContext);
}

void ObjectSelector::PerformRaycast(AbstractInput& input, AbstractEditorContext& editorContext)
{
#if 0
    if (pickMode == PICK_UI_ELEMENTS)
    {
        bool leftClick = mouseClick && input.mouseButtonPress[MOUSEB_LEFT];
        bool multiselect = input.qualifierDown[QUAL_CTRL];

        // Only interested in user-created UI elements
        if (elementAtPos !is null && elementAtPos !is editorUIElement && elementAtPos.GetElementEventSender() is editorUIElement)
        {
            ui.DebugDraw(elementAtPos);

            if (leftClick)
                SelectUIElement(elementAtPos, multiselect);
        }
        // If clicked on emptiness in non-multiselect mode, clear the selection
        else if (leftClick && !multiselect && ui.GetElementAt(pos) is null)
            hierarchyList.ClearSelection();

        return;
    }
#endif

    // Pick component
    Component* selectedComponent = nullptr;
    if (selectionMode_ == ObjectSelectionMode::Rigidbodies)
    {
        PhysicsWorld* physicsWorld = scene_->GetComponent<PhysicsWorld>();
        if (!physicsWorld)
            return;

        // If we are not running the actual physics update, refresh collisions before raycasting
        //if (!runUpdate) #TODO Fixme
        physicsWorld->UpdateCollisions();

        PhysicsRaycastResult result;
        physicsWorld->RaycastSingle(result, editorContext.GetMouseRay(), editorContext.GetCurrentCamera()->GetFarClip());
        if (result.body_)
            selectedComponent = result.body_;
    }
    else
    {
        Octree* octree = scene_->GetComponent<Octree>();
        if (!octree)
            return;

        static int pickModeDrawableFlags[3] = { DRAWABLE_GEOMETRY, DRAWABLE_LIGHT, DRAWABLE_ZONE };
        PODVector<RayQueryResult> result;
        RayOctreeQuery query(result, editorContext.GetMouseRay(), RAY_TRIANGLE, editorContext.GetCurrentCamera()->GetFarClip(),
            pickModeDrawableFlags[static_cast<int>(selectionMode_)], 0x7fffffff);
        octree->RaycastSingle(query);

        if (!result.Empty())
        {
            Drawable* drawable = result[0].drawable_;
            selectedComponent = ResolveRoutine(drawable);
        }
    }

    // Hover object
    selection_->SetHoveredObject(selectedComponent);

    // Preform selection
    if (selectedComponent)
    {
        if (controls_[TOGGLE_COMPONENT].IsPressed(input))
        {
            // Clear selection if there are nodes in existing selection
            const bool clearSelection = !selection_->GetSelectedNodes().Empty();
            selection_->SelectObject(selectedComponent, SelectionAction::Toggle, clearSelection);
        }

        if (controls_[TOGGLE_NODE].IsPressed(input))
        {
            // Clear selection if there are nodes in existing selection
            const bool clearSelection = !selection_->GetSelectedComponents().Empty();
            selection_->SelectObject(selectedComponent->GetNode(), SelectionAction::Toggle, clearSelection);
        }
    }

    if (controls_[SELECT_COMPONENT].IsPressed(input))
    {
        if (selectedComponent)
            selection_->SelectObject(selectedComponent, SelectionAction::Select, true);
        else
            selection_->ClearSelection();
    }

    if (controls_[SELECT_NODE].IsPressed(input))
    {
        if (selectedComponent)
            selection_->SelectObject(selectedComponent->GetNode(), SelectionAction::Select, true);
        else
            selection_->ClearSelection();
    }
}

Component* ObjectSelector::ResolveRoutine(Component* source)
{
    auto iter = selectionTransferringRoutines_.Find(source->GetTypeName());
    if (iter == selectionTransferringRoutines_.End())
        return source;

    const String& dest = iter->second_;
    Node* node = source->GetNode();
    while (node && !node->HasComponent(dest))
        node = node->GetParent();

    return node ? node->GetComponent(dest) : nullptr;
}

}

