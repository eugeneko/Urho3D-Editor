#include "HierarchyWindow.h"
#include "EditorEvents.h"
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Component.h>

namespace Urho3D
{

String HierarchyWindowItem::GetText()
{
    if (auto scene = dynamic_cast<Scene*>(object_))
        return scene->GetName().Empty() ? "Scene" : scene->GetName();
    else if (auto node = dynamic_cast<Node*>(object_))
        return node->GetName().Empty() ? "Node" : node->GetName();
    else if (auto component = dynamic_cast<Component*>(object_))
        return component->GetTypeName();
    return String::EMPTY;
}

//////////////////////////////////////////////////////////////////////////
HierarchyWindow::HierarchyWindow(AbstractWidgetStack* stack, Object* document)
    : Object(stack->GetContext())
    , stack_(stack)
    , document_(document)
{
    hierarchyList_ = stack_->CreateChild<AbstractHierarchyList>(document_);
    hierarchyList_->SetMultiselect(true);
    hierarchyList_->onItemClicked_ = [=](AbstractHierarchyListItem* item)
    {
        HandleListSelectionChanged();
    };
    SetScene(scene_);
}

HierarchyWindow::~HierarchyWindow()
{
    stack_->RemoveChild(document_);
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
        RemoveListItem(scene_);
    }
    scene_ = scene;
    if (scene_)
    {
        SubscribeToEvent(scene_, E_NODEADDED, URHO3D_HANDLER(HierarchyWindow, HandleNodeAdded));
        SubscribeToEvent(scene_, E_NODEREMOVED, URHO3D_HANDLER(HierarchyWindow, HandleNodeRemoved));
        SubscribeToEvent(scene_, E_COMPONENTADDED, URHO3D_HANDLER(HierarchyWindow, HandleComponentAdded));
        SubscribeToEvent(scene_, E_COMPONENTREMOVED, URHO3D_HANDLER(HierarchyWindow, HandleComponentRemoved));
        SubscribeToEvent(scene_, E_NODENAMECHANGED, URHO3D_HANDLER(HierarchyWindow, HandleNodeNameChanged));
        SubscribeToEvent(scene_, E_NODEENABLEDCHANGED, URHO3D_HANDLER(HierarchyWindow, HandleNodeEnabledChanged));
        SubscribeToEvent(scene_, E_COMPONENTENABLEDCHANGED, URHO3D_HANDLER(HierarchyWindow, HandleComponentEnabledChanged));
        UpdateListItem(scene_);
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

AbstractHierarchyListItem* HierarchyWindow::FindItem(Object* object)
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
    for (AbstractHierarchyListItem* item : hierarchyList_->GetSelection())
        if (HierarchyWindowItem* derivedItem = static_cast<HierarchyWindowItem*>(item))
            if (Object* object = derivedItem->GetObject())
                result.Insert(object);
}

AbstractHierarchyListItem* HierarchyWindow::CreateListItem(Object* object)
{
    auto item = new HierarchyWindowItem(object);
    objectsToItems_[WeakPtr<Object>(object)] = item;
    if (Node* node = dynamic_cast<Node*>(object))
    {
        for (Component* component : node->GetComponents())
            item->InsertChild(CreateListItem(component), M_MAX_UNSIGNED);
        for (Node* child : node->GetChildren())
            item->InsertChild(CreateListItem(child), M_MAX_UNSIGNED);
    }
    return item;
}

void HierarchyWindow::GetObjectParentAndIndex(Object* object, Object*& parent, unsigned& index)
{
    parent = nullptr;
    index = 0;

    if (auto node = dynamic_cast<Node*>(object))
    {
        Node* parentNode = node->GetParent();
        parent = parentNode;
        index = parentNode ? parentNode->GetChildren().IndexOf(SharedPtr<Node>(node)) : M_MAX_UNSIGNED;
    }
    else if (auto component = dynamic_cast<Component*>(object))
    {
        Node* parentNode = component->GetNode();
        parent = parentNode;
        index = parentNode->GetNumChildren() + parentNode->GetComponents().IndexOf(SharedPtr<Component>(component));
    }
}

void HierarchyWindow::UpdateListItem(Object* object)
{
    if (AbstractHierarchyListItem* oldObjectItem = FindItem(object))
        hierarchyList_->RemoveItem(oldObjectItem);

    Object* parentObject = nullptr;
    unsigned objectIndex = 0;
    GetObjectParentAndIndex(object, parentObject, objectIndex);
    AbstractHierarchyListItem* parentItem = FindItem(parentObject);
    AbstractHierarchyListItem* objectItem = CreateListItem(object);
    hierarchyList_->AddItem(objectItem, objectIndex, parentItem);
}

void HierarchyWindow::RemoveListItem(Object* object)
{
    AbstractHierarchyListItem* objectItem = FindItem(object);
    hierarchyList_->RemoveItem(objectItem);
}

void HierarchyWindow::HandleListSelectionChanged()
{
    suppressEditorSelectionChanges_ = true;
    selection_->SetSelection(GetSelectedObjects());
    suppressEditorSelectionChanges_ = false;
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
        if (AbstractHierarchyListItem* item = FindItem(object))
            hierarchyList_->DeselectItem(item);

    // Select new objects
    bool wasScrolled = false;
    for (Object* object : toSelect)
    {
        if (AbstractHierarchyListItem* item = FindItem(object))
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
    Node* node = dynamic_cast<Node*>(eventData[NodeAdded::P_NODE].GetPtr());
    UpdateListItem(node);
}

void HierarchyWindow::HandleNodeRemoved(StringHash eventType, VariantMap& eventData)
{
    Node* node = dynamic_cast<Node*>(eventData[NodeRemoved::P_NODE].GetPtr());
    RemoveListItem(node);
}

void HierarchyWindow::HandleComponentAdded(StringHash eventType, VariantMap& eventData)
{
    Component* component = dynamic_cast<Component*>(eventData[ComponentAdded::P_COMPONENT].GetPtr());
    UpdateListItem(component);
}

void HierarchyWindow::HandleComponentRemoved(StringHash eventType, VariantMap& eventData)
{
    Component* component = dynamic_cast<Component*>(eventData[ComponentRemoved::P_COMPONENT].GetPtr());
    RemoveListItem(component);
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

//////////////////////////////////////////////////////////////////////////
HierarchyWindow1::HierarchyWindow1(AbstractMainWindow* mainWindow)
    : Object(mainWindow->GetContext())
{
    dialog_ = mainWindow->AddDock(DockLocation::Left);
    dialog_->SetName("Hierarchy");

    stack_ = dialog_->CreateContent<AbstractWidgetStack>();
}

HierarchyWindow* HierarchyWindow1::GetDocument(Object* key)
{
    if (!documents_.Contains(key))
        documents_[key] = MakeShared<HierarchyWindow>(stack_, key);
    return documents_[key];
}

void HierarchyWindow1::RemoveDocument(Object* key)
{
    stack_->RemoveChild(key);
}

void HierarchyWindow1::SelectDocument(Object* key)
{
    stack_->SelectChild(key);
}

}
