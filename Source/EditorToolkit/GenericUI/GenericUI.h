#pragma once

#include "KeyBinding.h"
#include "AbstractInput.h"
#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/HashSet.h>
#include <Urho3D/Input/Input.h>

namespace Urho3D
{

class AbstractMainWindow;
class Scene;
class Node;
class Serializable;
class Selection;
class GenericDialog;

/// Generic UI widget clicked.
URHO3D_EVENT(E_GENERICWIDGETCLICKED, GenericWidgetClicked)
{
    URHO3D_PARAM(P_ELEMENT, Element);   // GenericWidget ptr
    URHO3D_PARAM(P_ITEM, Item);         // GenericWidget ptr (optional)
}

class GenericDocument : public Object
{
    URHO3D_OBJECT(GenericDocument, Object);

public:
    GenericDocument(Context* context) : Object(context) { }
};

enum class DialogLocationHint
{
    Undocked,
    DockLeft,
    DockRight,
    DockTop,
    DockBottom,
};

struct AbstractAction
{
    String id_;
    String text_;
    std::function<void()> actionCallback_;
    KeyBinding keyBinding_;
};

class GenericMenu
{
public:
    virtual GenericMenu* AddMenu(const String& name) = 0;
    virtual GenericMenu* AddAction(const String& name, const String& actionId) = 0;
};

// #TODO: Rename file
class GenericWidget : public Object
{
    URHO3D_OBJECT(GenericWidget, Object);

public:
    GenericWidget(AbstractMainWindow& mainWindow, GenericWidget* parent);
    GenericWidget* GetParent() const { return parent_; }

protected:
    AbstractMainWindow& mainWindow_;

private:
    GenericWidget* parent_ = nullptr;
};

class GenericDialog : public GenericWidget
{
    URHO3D_OBJECT(GenericDialog, GenericWidget);

public:
    GenericDialog(AbstractMainWindow& mainWindow, GenericWidget* parent) : GenericWidget(mainWindow, parent) { }
    GenericWidget* CreateBodyWidget(StringHash type);
    template <class T> T* CreateBodyWidget() { return dynamic_cast<T*>(CreateBodyWidget(T::GetTypeStatic())); }
    virtual void SetBodyWidget(GenericWidget* widget) = 0;
    virtual void SetName(const String& name) = 0;
};

class AbstractScrollRegion : public GenericWidget
{
    URHO3D_OBJECT(AbstractScrollRegion, GenericWidget);

public:
    AbstractScrollRegion(AbstractMainWindow& mainWindow, GenericWidget* parent) : GenericWidget(mainWindow, parent) { }

    virtual void SetDynamicWidth(bool dynamicWidth) = 0;

    GenericWidget* CreateContent(StringHash type);
    template <class T> T* CreateContent() { return dynamic_cast<T*>(CreateContent(T::GetTypeStatic())); }

private:
    virtual bool SetContent(GenericWidget* content) = 0;

private:
    SharedPtr<GenericWidget> content_;

};

class AbstractLayout : public GenericWidget
{
    URHO3D_OBJECT(AbstractLayout, GenericWidget);

public:
    AbstractLayout(AbstractMainWindow& mainWindow, GenericWidget* parent) : GenericWidget(mainWindow, parent) { }

    GenericWidget* CreateCellWidget(StringHash type, unsigned row, unsigned column);
    template <class T> T* CreateCellWidget(unsigned row, unsigned column) { return dynamic_cast<T*>(CreateCellWidget(T::GetTypeStatic(), row, column)); }

    GenericWidget* CreateRowWidget(StringHash type, unsigned row);
    template <class T> T* CreateRowWidget(unsigned row) { return dynamic_cast<T*>(CreateRowWidget(T::GetTypeStatic(), row)); }

private:
    virtual bool SetCellWidget(unsigned row, unsigned column, GenericWidget* childWidget) = 0;
    virtual bool SetRowWidget(unsigned row, GenericWidget* childWidget) = 0;

};

class AbstractButton : public GenericWidget
{
    URHO3D_OBJECT(AbstractButton, GenericWidget);

public:
    AbstractButton(AbstractMainWindow& mainWindow, GenericWidget* parent) : GenericWidget(mainWindow, parent) { }
    virtual AbstractButton& SetText(const String& text) = 0;
};

class AbstractText : public GenericWidget
{
    URHO3D_OBJECT(AbstractText, GenericWidget);

public:
    AbstractText(AbstractMainWindow& mainWindow, GenericWidget* parent) : GenericWidget(mainWindow, parent) { }
    virtual AbstractText& SetText(const String& text) = 0;
    virtual AbstractText& SetFixedWidth(bool fixedSize) = 0;
};

class AbstractLineEdit : public GenericWidget
{
    URHO3D_OBJECT(AbstractLineEdit, GenericWidget);

public:
    AbstractLineEdit(AbstractMainWindow& mainWindow, GenericWidget* parent) : GenericWidget(mainWindow, parent) { }
    virtual AbstractLineEdit& SetText(const String& text) = 0;
};

class GenericHierarchyListItem : public Object
{
    URHO3D_OBJECT(GenericHierarchyListItem, Object);

public:
    GenericHierarchyListItem(Context* context) : Object(context) { }
    void SetParent(GenericHierarchyListItem* parent) { parent_ = parent; }
    void SetInternalPointer(Object* internalPointer) { internalPointer_ = internalPointer; }
    Object* GetInternalPointer() const { return internalPointer_; }

    void InsertChild(GenericHierarchyListItem* item, unsigned index);
    void RemoveChild(unsigned index);
    GenericHierarchyListItem* GetParent() const { return parent_; }
    unsigned GetNumChildren() const { return children_.Size(); }
    GenericHierarchyListItem* GetChild(unsigned index) const { return index < children_.Size() ? children_[index] : nullptr; }
    int GetIndex();

    virtual String GetText() { return String::EMPTY; }

private:
    GenericHierarchyListItem* parent_ = nullptr;
    Object* internalPointer_ = nullptr;
    Vector<SharedPtr<GenericHierarchyListItem>> children_;
};

class GenericHierarchyList : public GenericWidget
{
    URHO3D_OBJECT(GenericHierarchyList, GenericWidget);

public:
    using ItemVector = PODVector<GenericHierarchyListItem*>;
    GenericHierarchyList(AbstractMainWindow& mainWindow, GenericWidget* parent) : GenericWidget(mainWindow, parent) { }
    virtual void AddItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent) = 0;
    virtual void SelectItem(GenericHierarchyListItem* item) = 0;
    virtual void DeselectItem(GenericHierarchyListItem* item) = 0;
    virtual void GetSelection(ItemVector& result) = 0;
    ItemVector GetSelection() { ItemVector result; GetSelection(result); return result; }
};

class AbstractMainWindow
{
public:
    SharedPtr<GenericWidget> CreateWidget(StringHash type, GenericWidget* parent);
    virtual GenericDialog* AddDialog(DialogLocationHint hint = DialogLocationHint::Undocked) = 0;
    virtual void AddAction(const AbstractAction& actionDesc) = 0;
    virtual GenericMenu* AddMenu(const String& name) = 0;

    virtual Context* GetContext() = 0;
    virtual AbstractInput* GetInput() = 0;

    template <class T> void AddAction(const String& id, KeyBinding keyBinding, T function)
    {
        AddAction({ id, "", function, keyBinding });
    }

private:
    virtual SharedPtr<GenericWidget> CreateScrollRegion(GenericWidget* parent);
    virtual SharedPtr<GenericWidget> CreateLayout(GenericWidget* parent);
    virtual SharedPtr<GenericWidget> CreateButton(GenericWidget* parent);
    virtual SharedPtr<GenericWidget> CreateText(GenericWidget* parent);
    virtual SharedPtr<GenericWidget> CreateLineEdit(GenericWidget* parent);
    virtual SharedPtr<GenericWidget> CreateHierarchyList(GenericWidget* parent);
};

}

#define URHO3D_IMPLEMENT_WIDGET_FACTORY(factory, implementation) \
    SharedPtr<GenericWidget> factory(GenericWidget* parent) override { return MakeShared<implementation>(*this, parent); }

