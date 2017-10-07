#include "HierarchyWindow.h"
#include "EditorEvents.h"
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

String HierarchyWindowItem::GetText()
{
    if (Scene* scene = dynamic_cast<Scene*>(object_))
        return scene->GetName().Empty() ? "Scene" : scene->GetName();
    else if (Node* node = dynamic_cast<Node*>(object_))
        return node->GetName().Empty() ? "Node" : node->GetName();
    return String::EMPTY;
}

//////////////////////////////////////////////////////////////////////////
HierarchyWindow::HierarchyWindow(AbstractUI& ui)
    : Object(ui.GetContext())
{
    GenericMainWindow* mainWindow = ui.GetMainWindow();
    dialog_ = mainWindow->AddDialog(DialogLocationHint::DockLeft);
    dialog_->SetName("Hierarchy");

    hierarchyList_ = dialog_->CreateBodyWidget<GenericHierarchyList>();
    SubscribeToEvent(hierarchyList_, E_GENERICWIDGETCLICKED, URHO3D_HANDLER(HierarchyWindow, HandleListSelectionChanged));
    SetScene(scene_);
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
        SubscribeToEvent(E_EDITORSELECTIONCHANGED, URHO3D_HANDLER(HierarchyWindow, HandleEditorSelectionChanged));
}

Selection::ObjectSet HierarchyWindow::GetSelectedObjects()
{
    Selection::ObjectSet result;
    GatherHierarchyListSelections(result);
    return result;
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
        if (HierarchyWindowItem* derivedItem = static_cast<HierarchyWindowItem*>(item))
            if (Object* object = derivedItem->GetObject())
                result.Insert(object);
}

GenericHierarchyListItem* HierarchyWindow::CreateListItem(Object* object)
{
    auto item = new HierarchyWindowItem(object);
    objectsToItems_[WeakPtr<Object>(object)] = item;
    if (Node* node = dynamic_cast<Node*>(object))
    {
        for (Node* child : node->GetChildren())
            item->InsertChild(CreateListItem(child), M_MAX_UNSIGNED);
    }
    return item;
}

void HierarchyWindow::AddNode(Node* node)
{
    Node* parent = node->GetParent();
    GenericHierarchyListItem* parentItem = FindItem(parent);
    GenericHierarchyListItem* objectItem = CreateListItem(node);

    hierarchyList_->AddItem(objectItem, M_MAX_UNSIGNED, parentItem);
}

void HierarchyWindow::HandleListSelectionChanged(StringHash eventType, VariantMap& eventData)
{
    if (eventData[GenericWidgetClicked::P_ITEM].GetPtr() != nullptr)
    {
        suppressEditorSelectionChanges_ = true;
        selection_->SetSelection(GetSelectedObjects());
        suppressEditorSelectionChanges_ = false;
    }
}

void HierarchyWindow::HandleEditorSelectionChanged(StringHash /*eventType*/, VariantMap& /*eventData*/)
{
    if (suppressEditorSelectionChanges_)
        return;

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
