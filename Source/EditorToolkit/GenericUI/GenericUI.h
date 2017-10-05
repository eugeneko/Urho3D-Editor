#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/HashSet.h>

namespace Urho3D
{

class AbstractUI;
class Scene;
class Node;
class Serializable;
class Selection;

/// Generic UI widget clicked.
URHO3D_EVENT(E_GENERICWIDGETCLICKED, GenericWidgetClicked)
{
    URHO3D_PARAM(P_ELEMENT, Element);   // GenericWidget ptr
    URHO3D_PARAM(P_ITEM, Item);         // GenericWidget ptr (optional)
}

class GenericMainWindow : public Object
{
    URHO3D_OBJECT(GenericMainWindow, Object);
public:
    GenericMainWindow(Context* context) : Object(context) { }
};

// #TODO: Rename file
class GenericWidget : public Object
{
    URHO3D_OBJECT(GenericWidget, Object);

public:
    GenericWidget(Context* context) : Object(context) { }
    void SetHost(AbstractUI* ui) { ui_ = ui; OnHostInitialized(); }
    GenericWidget* CreateChild(StringHash type);
    template <class T> T* CreateChild() { return dynamic_cast<T*>(CreateChild(T::GetTypeStatic())); }

protected:
    virtual void OnHostInitialized() { }
    virtual void OnChildAdded(GenericWidget* widget);

private:
    AbstractUI* ui_ = nullptr;
    Vector<SharedPtr<GenericWidget>> children_;

};

class GenericDialog : public GenericWidget
{
    URHO3D_OBJECT(GenericDialog, GenericWidget);

public:
    GenericDialog(Context* context) : GenericWidget(context) { }
    virtual void SetName(const String& name) = 0;
};

class GenericHierarchyListItem : public GenericWidget
{
    URHO3D_OBJECT(GenericHierarchyListItem, GenericWidget);

public:
    GenericHierarchyListItem(Context* context) : GenericWidget(context) { }
    virtual void SetText(const String& text) = 0;
    void SetObject(Object* object) { object_ = object; }
    Object* GetObject() const { return object_; }

private:
    Object* object_ = nullptr;
};

class GenericHierarchyList : public GenericWidget
{
    URHO3D_OBJECT(GenericHierarchyList, GenericWidget);

public:
    using ItemVector = PODVector<GenericHierarchyListItem*>;
    GenericHierarchyList(Context* context) : GenericWidget(context) { }
    virtual void SelectItem(GenericHierarchyListItem* item) = 0;
    virtual void DeselectItem(GenericHierarchyListItem* item) = 0;
    virtual void GetSelection(ItemVector& result) = 0;
    ItemVector GetSelection() { ItemVector result; GetSelection(result); return result; }
};

class AbstractUI
{
public:
    GenericWidget* CreateWidget(StringHash type);
    template <class T> T* CreateWidget() { return dynamic_cast<T*>(CreateWidget(T::GetTypeStatic())); }
    void SetMainWindow(GenericMainWindow* mainWindow) { mainWindow_ = mainWindow; }
    GenericMainWindow* GetMainWindow() { return mainWindow_; }

private:
    virtual GenericWidget* CreateWidgetImpl(StringHash type) = 0;

private:
    UniquePtr<AbstractUI> defaultHost_;
    SharedPtr<GenericMainWindow> mainWindow_;
};

}
