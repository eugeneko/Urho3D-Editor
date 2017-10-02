#include "HierarchyWindow.h"
#include "EditorEvents.h"
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

HierarchyWindow::HierarchyWindow(Context* context) : Object(context)
{
    CreateWidgets(GetSubsystem<GenericUI>()->GetDefaultHost());
}

void HierarchyWindow::SetScene(Scene* scene)
{
    if (scene_)
    {
        UnsubscribeFromEvent(scene_, E_NODEADDED);
        UnsubscribeFromEvent(scene_, E_NODEREMOVED);
        UnsubscribeFromEvent(scene_, E_COMPONENTADDED);
        UnsubscribeFromEvent(scene_, E_COMPONENTREMOVED);
        UnsubscribeFromEvent(scene_, E_NODENAMECHANGED);
        UnsubscribeFromEvent(scene_, E_NODEENABLEDCHANGED);
        UnsubscribeFromEvent(scene_, E_COMPONENTENABLEDCHANGED);
        //unsigned int index = GetListIndex(scene_);
        //UpdateHierarchyItem(index, NULL, NULL);
    }
    scene_ = scene;
    if (scene_)
    {
        //UpdateHierarchyItem(scene_);
        SubscribeToEvent(scene_, E_NODEADDED, URHO3D_HANDLER(HierarchyWindow, HandleNodeAdded));
        SubscribeToEvent(scene_, E_NODEREMOVED, URHO3D_HANDLER(HierarchyWindow, HandleNodeRemoved));
        SubscribeToEvent(scene_, E_COMPONENTADDED, URHO3D_HANDLER(HierarchyWindow, HandleComponentAdded));
        SubscribeToEvent(scene_, E_COMPONENTREMOVED, URHO3D_HANDLER(HierarchyWindow, HandleComponentRemoved));
        SubscribeToEvent(scene_, E_NODENAMECHANGED, URHO3D_HANDLER(HierarchyWindow, HandleNodeNameChanged));
        SubscribeToEvent(scene_, E_NODEENABLEDCHANGED, URHO3D_HANDLER(HierarchyWindow, HandleNodeEnabledChanged));
        SubscribeToEvent(scene_, E_COMPONENTENABLEDCHANGED, URHO3D_HANDLER(HierarchyWindow, HandleComponentEnabledChanged));
        AddNode(scene_);
    }
}

void HierarchyWindow::SetSelection(Selection* selection)
{
    if (selection_)
        UnsubscribeFromEvent(E_EDITORSELECTIONCHANGED);
    selection_ = selection;
    if (selection_)
        SubscribeToEvent(E_EDITORSELECTIONCHANGED, URHO3D_HANDLER(HierarchyWindow, HandleSelectionChanged));
}

Selection::ObjectSet HierarchyWindow::GetSelectedObjects()
{
    Selection::ObjectSet result;
    GatherHierarchyListSelections(result);
    return result;
}

void HierarchyWindow::CreateWidgets(GenericUIHost* host)
{
    dialog_ = host->CreateWidget<GenericDialog>();
    dialog_->SetName("Hierarchy");

    hierarchyList_ = dialog_->CreateChild<GenericHierarchyList>();
    SetScene(scene_);
}

GenericHierarchyListItem* HierarchyWindow::FindItem(Object* object)
{
    if (object)
        return objectsToItems_[WeakPtr<Object>(object)];
    else
        return nullptr;
}

void HierarchyWindow::Subtract(const Selection::ObjectSet& lhs, const Selection::ObjectSet& rhs, Selection::ObjectSet& result) const
{
    result.Clear();
    for (Object* object : lhs)
        if (!rhs.Contains(object))
            result.Insert(object);
}

void HierarchyWindow::GatherHierarchyListSelections(Selection::ObjectSet& result) const
{
    // TODO: Cache
    for (GenericHierarchyListItem* item : hierarchyList_->GetSelection())
        if (Object* object = item->GetObject())
            result.Insert(object);
}

void HierarchyWindow::AddNode(Node* node)
{
    Node* parent = node->GetParent();
    GenericHierarchyListItem* parentItem = FindItem(parent);
    GenericHierarchyListItem* objectItem = parentItem
        ? parentItem->CreateChild<GenericHierarchyListItem>()
        : hierarchyList_->CreateChild<GenericHierarchyListItem>();

    objectsToItems_[WeakPtr<Object>(node)] = objectItem;

    objectItem->SetObject(node);
    if (Scene* scene = dynamic_cast<Scene*>(node))
        objectItem->SetText(scene->GetName().Empty() ? "Scene" : scene->GetName());
    else
        objectItem->SetText(node->GetName().Empty() ? "Node" : node->GetName());

    for (Node* child : node->GetChildren())
        AddNode(child);
}

void HierarchyWindow::HandleSelectionChanged(StringHash /*eventType*/, VariantMap& /*eventData*/)
{
    // TODO: Cache
    Selection::ObjectSet oldSelection = GetSelectedObjects();
    Selection::ObjectSet newSelection = selection_->GetSelected();
    Selection::ObjectSet toSelect;
    Subtract(newSelection, oldSelection, toSelect);
    Selection::ObjectSet toDeselect;
    Subtract(oldSelection, newSelection, toDeselect);

    // Deselect old objects
    for (Object* object : toDeselect)
        if (GenericHierarchyListItem* item = FindItem(object))
            hierarchyList_->DeselectItem(item);

    // Select new objects
    bool wasScrolled = false;
    for (Object* object : toSelect)
    {
        if (GenericHierarchyListItem* item = FindItem(object))
        {
            hierarchyList_->SelectItem(item);
            if (!wasScrolled)
            {
                wasScrolled = true;
                // TODO: Add scroll
                // treeView_->scrollTo(index);
            }
        }
    }
}

void HierarchyWindow::HandleNodeAdded(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleNodeRemoved(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleComponentAdded(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleComponentRemoved(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleNodeNameChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleNodeEnabledChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleComponentEnabledChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleUIElementNameChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleUIElementVisibilityChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleUIElementAttributeChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleUIElementAdded(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleUIElementRemoved(StringHash eventType, VariantMap& eventData)
{

}

}