#pragma once

#include "Selection.h"
#include "../AbstractUI/AbstractUI.h"
#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/HashMap.h>

namespace Urho3D
{

class AbstractMainWindow;
class Scene;
class Node;

class HierarchyWindowItem : public AbstractHierarchyListItem
{
public:
    HierarchyWindowItem(Object* object) : AbstractHierarchyListItem(object->GetContext()), object_(object) { }
    Object* GetObject() { return object_; }

    String GetText() override;

private:
    Object* object_ = nullptr;
};

class Hierarchy : public Object
{
    URHO3D_OBJECT(Hierarchy, Object);

public:
    Hierarchy(AbstractWidgetStack* stack, Object* document);
    ~Hierarchy() override;
    void RefreshSelection();
    void SetScene(Scene* scene);
    void SetSelection(Selection* selection);
    const Selection::ObjectVector& GetSelectedObjects() { return cachedSelection_; }

private:
    AbstractHierarchyListItem* FindItem(Object* object);
    /// Subtract right set from left one.
    void Subtract(const Selection::ObjectVector& lhs, const Selection::ObjectSet& rhs, Selection::ObjectSet& result) const;
    /// Gather selection from hierarchy list.
    void CacheSelection();
    AbstractHierarchyListItem* CreateListItem(Object* object);
    void GetObjectParentAndIndex(Object* object, Object*& parent, unsigned& index);
    void UpdateListItem(Object* object);
    void RemoveListItem(Object* object);

    // @name Editor and UI Events
    // @{

    void HandleListSelectionChanged();
    void HandleEditorSelectionChanged();

    // @}

    // @name Scene Events
    // @{

    void HandleNodeAdded(StringHash eventType, VariantMap& eventData);
    void HandleNodeRemoved(StringHash eventType, VariantMap& eventData);
    void HandleComponentAdded(StringHash eventType, VariantMap& eventData);
    void HandleComponentRemoved(StringHash eventType, VariantMap& eventData);
    void HandleNodeNameChanged(StringHash eventType, VariantMap& eventData);
    void HandleNodeEnabledChanged(StringHash eventType, VariantMap& eventData);
    void HandleComponentEnabledChanged(StringHash eventType, VariantMap& eventData);

    // @}

    // @name UI Events
    // @{

    void HandleUIElementNameChanged(StringHash eventType, VariantMap& eventData);
    void HandleUIElementVisibilityChanged(StringHash eventType, VariantMap& eventData);
    void HandleUIElementAttributeChanged(StringHash eventType, VariantMap& eventData);
    void HandleUIElementAdded(StringHash eventType, VariantMap& eventData);
    void HandleUIElementRemoved(StringHash eventType, VariantMap& eventData);

    // @}

private:
    AbstractWidgetStack* stack_ = nullptr;
    Object* document_ = nullptr;
    AbstractHierarchyList* hierarchyList_ = nullptr;
    SharedPtr<Scene> scene_;
    SharedPtr<Selection> selection_;
    HashMap<WeakPtr<Object>, WeakPtr<AbstractHierarchyListItem>> objectsToItems_;

    bool suppressEditorSelectionChanges_ = false;
    Selection::ObjectVector cachedSelection_;
    Selection::ObjectSet cachedSelectionSet_;
};

class HierarchyWindow : public Object
{
    URHO3D_OBJECT(HierarchyWindow, Object);

public:
    HierarchyWindow(AbstractMainWindow* mainWindow);
    Hierarchy* GetDocument(Object* key);
    void RemoveDocument(Object* key);
    void SelectDocument(Object* key);

private:
    AbstractDock* dialog_ = nullptr;
    AbstractWidgetStack* stack_ = nullptr;
    HashMap<Object*, SharedPtr<Hierarchy>> documents_;
};

}
