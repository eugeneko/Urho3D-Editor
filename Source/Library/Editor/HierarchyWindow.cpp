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
Hierarchy::Hierarchy(AbstractWidgetStack* stack, Object* document)
    : Object(stack->GetContext())
    , stack_(stack)
    , document_(document)
{
    hierarchyList_ = stack_->CreateChild<AbstractHierarchyList>(document_);
    hierarchyList_->SetMultiselect(true);
    hierarchyList_->onItemClicked_ = [=](AbstractHierarchyListItem* item)
    {
        HandleListSelectionChanged();
        itemContextMenu_->Show();
    };
    SetScene(scene_);

    itemContextMenu_ = stack->GetMainWindow()->CreateContextMenu(AbstractMenuItem({
        AbstractMenuItem("First"),
        AbstractMenuItem("Second"),
        AbstractMenuItem("Third"),
    }));
}

Hierarchy::~Hierarchy()
{
    stack_->RemoveChild(document_);
}

void Hierarchy::RefreshSelection()
{
    HandleEditorSelectionChanged();
}

void Hierarchy::SetScene(Scene* scene)
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
        SubscribeToEvent(scene_, E_NODEADDED, URHO3D_HANDLER(Hierarchy, HandleNodeAdded));
        SubscribeToEvent(scene_, E_NODEREMOVED, URHO3D_HANDLER(Hierarchy, HandleNodeRemoved));
        SubscribeToEvent(scene_, E_COMPONENTADDED, URHO3D_HANDLER(Hierarchy, HandleComponentAdded));
        SubscribeToEvent(scene_, E_COMPONENTREMOVED, URHO3D_HANDLER(Hierarchy, HandleComponentRemoved));
        SubscribeToEvent(scene_, E_NODENAMECHANGED, URHO3D_HANDLER(Hierarchy, HandleNodeNameChanged));
        SubscribeToEvent(scene_, E_NODEENABLEDCHANGED, URHO3D_HANDLER(Hierarchy, HandleNodeEnabledChanged));
        SubscribeToEvent(scene_, E_COMPONENTENABLEDCHANGED, URHO3D_HANDLER(Hierarchy, HandleComponentEnabledChanged));
        UpdateListItem(scene_);
    }
}

void Hierarchy::SetSelection(Selection* selection)
{
    selection_ = selection;
}

AbstractHierarchyListItem* Hierarchy::FindItem(Object* object)
{
    if (object)
        return objectsToItems_[WeakPtr<Object>(object)];
    else
        return nullptr;
}

void Hierarchy::Subtract(const Selection::ObjectVector& lhs, const Selection::ObjectSet& rhs, Selection::ObjectSet& result) const
{
    result.Clear();
    for (const WeakPtr<Object>& object : lhs)
        if (!rhs.Contains(object))
            result.Insert(object);
}

void Hierarchy::CacheSelection()
{
    cachedSelection_.Clear();
    cachedSelectionSet_.Clear();
    for (AbstractHierarchyListItem* item : hierarchyList_->GetSelection())
    {
        if (HierarchyWindowItem* derivedItem = static_cast<HierarchyWindowItem*>(item))
        {
            if (Object* object = derivedItem->GetObject())
            {
                WeakPtr<Object> weakObject(object);
                cachedSelection_.Push(weakObject);
                cachedSelectionSet_.Insert(weakObject);
            }
        }
    }
}

AbstractHierarchyListItem* Hierarchy::CreateListItem(Object* object)
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

void Hierarchy::GetObjectParentAndIndex(Object* object, Object*& parent, unsigned& index)
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

void Hierarchy::UpdateListItem(Object* object)
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

void Hierarchy::RemoveListItem(Object* object)
{
    AbstractHierarchyListItem* objectItem = FindItem(object);
    hierarchyList_->RemoveItem(objectItem);
}

void Hierarchy::HandleListSelectionChanged()
{
    suppressEditorSelectionChanges_ = true;
    CacheSelection();
    selection_->SetSelection(cachedSelection_);
    suppressEditorSelectionChanges_ = false;
}

void Hierarchy::HandleEditorSelectionChanged()
{
    if (suppressEditorSelectionChanges_)
        return;

    CacheSelection();

    Selection::ObjectSet toSelect;
    Subtract(selection_->GetObjects(), cachedSelectionSet_, toSelect);

    Selection::ObjectSet toDeselect;
    Subtract(cachedSelection_, selection_->GetObjectsSet(), toDeselect);

    // Deselect old objects
    for (Object* object : toDeselect)
        if (AbstractHierarchyListItem* item = FindItem(object))
            hierarchyList_->DeselectItem(item);

    // Select new objects
    for (Object* object : toSelect)
    {
        if (AbstractHierarchyListItem* item = FindItem(object))
        {
            hierarchyList_->ExpandItem(item);
            hierarchyList_->SelectItem(item);
        }
    }

    CacheSelection();
}

void Hierarchy::HandleNodeAdded(StringHash eventType, VariantMap& eventData)
{
    Node* node = dynamic_cast<Node*>(eventData[NodeAdded::P_NODE].GetPtr());
    UpdateListItem(node);
}

void Hierarchy::HandleNodeRemoved(StringHash eventType, VariantMap& eventData)
{
    Node* node = dynamic_cast<Node*>(eventData[NodeRemoved::P_NODE].GetPtr());
    RemoveListItem(node);
}

void Hierarchy::HandleComponentAdded(StringHash eventType, VariantMap& eventData)
{
    Component* component = dynamic_cast<Component*>(eventData[ComponentAdded::P_COMPONENT].GetPtr());
    UpdateListItem(component);
}

void Hierarchy::HandleComponentRemoved(StringHash eventType, VariantMap& eventData)
{
    Component* component = dynamic_cast<Component*>(eventData[ComponentRemoved::P_COMPONENT].GetPtr());
    RemoveListItem(component);
}

void Hierarchy::HandleNodeNameChanged(StringHash eventType, VariantMap& eventData)
{

}

void Hierarchy::HandleNodeEnabledChanged(StringHash eventType, VariantMap& eventData)
{

}

void Hierarchy::HandleComponentEnabledChanged(StringHash eventType, VariantMap& eventData)
{

}

void Hierarchy::HandleUIElementNameChanged(StringHash eventType, VariantMap& eventData)
{

}

void Hierarchy::HandleUIElementVisibilityChanged(StringHash eventType, VariantMap& eventData)
{

}

void Hierarchy::HandleUIElementAttributeChanged(StringHash eventType, VariantMap& eventData)
{

}

void Hierarchy::HandleUIElementAdded(StringHash eventType, VariantMap& eventData)
{

}

void Hierarchy::HandleUIElementRemoved(StringHash eventType, VariantMap& eventData)
{

}

//////////////////////////////////////////////////////////////////////////
HierarchyWindow::HierarchyWindow(AbstractMainWindow* mainWindow)
    : Object(mainWindow->GetContext())
{
    dialog_ = mainWindow->AddDock(DockLocation::Left);
    dialog_->SetName("Hierarchy");

    stack_ = dialog_->CreateContent<AbstractWidgetStack>();
}

Hierarchy* HierarchyWindow::GetDocument(Object* key)
{
    if (!documents_.Contains(key))
        documents_[key] = MakeShared<Hierarchy>(stack_, key);
    return documents_[key];
}

void HierarchyWindow::RemoveDocument(Object* key)
{
    stack_->RemoveChild(key);
}

void HierarchyWindow::SelectDocument(Object* key)
{
    stack_->SelectChild(key);
}

}
