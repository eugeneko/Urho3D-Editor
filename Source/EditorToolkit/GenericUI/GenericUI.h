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
    GenericWidget(AbstractMainWindow& mainWindow);

    void SetParent(GenericWidget* parent);
    GenericWidget* GetParent() const { return parent_; }

    template <class T> void SetInternalHandle(T pointer) { internalPointer_ = MakeCustomValue(pointer); }
    template <class T> T GetInternalHandle() const { return internalPointer_.GetCustom<T>(); }

    AbstractMainWindow* GetMainWindow() const { return &mainWindow_; }

private:
    /// Called when widget is attached to the root.
    virtual void OnParentSet() { }

protected:
    AbstractMainWindow& mainWindow_;

private:
    GenericWidget* parent_ = nullptr;
    Variant internalPointer_;
    bool attachedToRoot_ = false;
};

class GenericDialog : public GenericWidget
{
    URHO3D_OBJECT(GenericDialog, GenericWidget);

public:
    GenericDialog(AbstractMainWindow& mainWindow) : GenericWidget(mainWindow) { }

    GenericWidget* CreateContent(StringHash type);
    template <class T> T* CreateContent() { return dynamic_cast<T*>(CreateContent(T::GetTypeStatic())); }

    virtual void SetName(const String& name) = 0;

private:
    bool SetContent(GenericWidget* content);

    virtual bool DoSetContent(GenericWidget* content) = 0;

private:
    SharedPtr<GenericWidget> content_;

};

class AbstractDummyWidget : public GenericWidget
{
    URHO3D_OBJECT(AbstractDummyWidget, GenericWidget);

public:
    AbstractDummyWidget(AbstractMainWindow& mainWindow) : GenericWidget(mainWindow) { }
};

class AbstractScrollArea : public GenericWidget
{
    URHO3D_OBJECT(AbstractScrollArea, GenericWidget);

public:
    AbstractScrollArea(AbstractMainWindow& mainWindow) : GenericWidget(mainWindow) { }

    virtual void SetDynamicWidth(bool dynamicWidth) = 0;

    GenericWidget* CreateContent(StringHash type);
    template <class T> T* CreateContent() { return dynamic_cast<T*>(CreateContent(T::GetTypeStatic())); }

private:
    bool SetContent(GenericWidget* content);

    virtual bool DoSetContent(GenericWidget* content) = 0;

private:
    SharedPtr<GenericWidget> content_;

};

class AbstractLayout : public GenericWidget
{
    URHO3D_OBJECT(AbstractLayout, GenericWidget);

public:
    AbstractLayout(AbstractMainWindow& mainWindow) : GenericWidget(mainWindow) { }

    GenericWidget* CreateCell(StringHash type, unsigned row, unsigned column);
    template <class T> T* CreateCell(unsigned row, unsigned column) { return dynamic_cast<T*>(CreateCell(T::GetTypeStatic(), row, column)); }

    GenericWidget* CreateRow(StringHash type, unsigned row);
    template <class T> T* CreateRow(unsigned row) { return dynamic_cast<T*>(CreateRow(T::GetTypeStatic(), row)); }

    void RemoveRow(unsigned row);
    void RemoveAllChildren();

private:
    bool SetCell(GenericWidget* cell, unsigned row, unsigned column);
    bool SetRow(GenericWidget* cell, unsigned row);

    virtual bool DoSetCell(unsigned row, unsigned column, GenericWidget* child) = 0;
    virtual bool DoSetRow(unsigned row, GenericWidget* child) = 0;
    virtual void DoRemoveChild(GenericWidget* child) = 0;

protected:
    enum class RowType
    {
        EmptyRow,
        SimpleRow,
        MultiCellRow
    };

    struct RowData
    {
        RowType type_ = RowType::EmptyRow;
        Vector<SharedPtr<GenericWidget>> columns_;
    };

    bool EnsureRow(unsigned row, RowType type);
    bool EnsureCell(unsigned row, unsigned column, RowType type);

    Vector<RowData> rows_;

};

class AbstractCollapsiblePanel : public GenericWidget
{
    URHO3D_OBJECT(AbstractCollapsiblePanel, GenericWidget);

public:
    AbstractCollapsiblePanel(AbstractMainWindow& mainWindow) : GenericWidget(mainWindow) { }

    virtual void SetHeaderText(const String& text) = 0;
    virtual void SetExpanded(bool expanded) = 0;

    GenericWidget* CreateHeaderPrefix(StringHash type);
    template <class T> T* CreateHeaderPrefix() { return dynamic_cast<T*>(CreateHeaderPrefix(T::GetTypeStatic())); }

    GenericWidget* CreateHeaderSuffix(StringHash type);
    template <class T> T* CreateHeaderSuffix() { return dynamic_cast<T*>(CreateHeaderSuffix(T::GetTypeStatic())); }

    GenericWidget* CreateBody(StringHash type);
    template <class T> T* CreateBody() { return dynamic_cast<T*>(CreateBody(T::GetTypeStatic())); }

private:
    virtual bool SetHeaderPrefix(GenericWidget* header);
    virtual bool SetHeaderSuffix(GenericWidget* header);
    virtual bool SetBody(GenericWidget* body);

    virtual bool DoSetHeaderPrefix(GenericWidget* header) = 0;
    virtual bool DoSetHeaderSuffix(GenericWidget* header) = 0;
    virtual bool DoSetBody(GenericWidget* body) = 0;

private:
    SharedPtr<GenericWidget> headerPrefix_;
    SharedPtr<GenericWidget> headerSuffix_;
    SharedPtr<GenericWidget> body_;

};

class AbstractButton : public GenericWidget
{
    URHO3D_OBJECT(AbstractButton, GenericWidget);

public:
    AbstractButton(AbstractMainWindow& mainWindow) : GenericWidget(mainWindow) { }
    virtual AbstractButton& SetText(const String& text) = 0;
};

class AbstractText : public GenericWidget
{
    URHO3D_OBJECT(AbstractText, GenericWidget);

public:
    AbstractText(AbstractMainWindow& mainWindow) : GenericWidget(mainWindow) { }
    virtual AbstractText& SetText(const String& text) = 0;
    virtual unsigned GetTextWidth() const = 0;
};

class AbstractLineEdit : public GenericWidget
{
    URHO3D_OBJECT(AbstractLineEdit, GenericWidget);

public:
    AbstractLineEdit(AbstractMainWindow& mainWindow) : GenericWidget(mainWindow) { }
    virtual AbstractLineEdit& SetText(const String& text) = 0;

public:
    std::function<void(const String& value)> onTextEdited_;
    std::function<void(const String& value)> onTextFinished_;
};

class AbstractCheckBox : public GenericWidget
{
    URHO3D_OBJECT(AbstractCheckBox, GenericWidget);

public:
    AbstractCheckBox(AbstractMainWindow& mainWindow) : GenericWidget(mainWindow) { }
    virtual AbstractCheckBox& SetChecked(bool checked) = 0;
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
    GenericHierarchyList(AbstractMainWindow& mainWindow) : GenericWidget(mainWindow) { }
    virtual void AddItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent) = 0;
    virtual void SelectItem(GenericHierarchyListItem* item) = 0;
    virtual void DeselectItem(GenericHierarchyListItem* item) = 0;
    virtual void GetSelection(ItemVector& result) = 0;
    ItemVector GetSelection() { ItemVector result; GetSelection(result); return result; }
};

class AbstractMainWindow
{
public:
    SharedPtr<GenericWidget> CreateWidget(StringHash type);
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
    virtual SharedPtr<GenericWidget> CreateDummyWidget();
    virtual SharedPtr<GenericWidget> CreateScrollArea();
    virtual SharedPtr<GenericWidget> CreateLayout();
    virtual SharedPtr<GenericWidget> CreateCollapsiblePanel();
    virtual SharedPtr<GenericWidget> CreateButton();
    virtual SharedPtr<GenericWidget> CreateText();
    virtual SharedPtr<GenericWidget> CreateLineEdit();
    virtual SharedPtr<GenericWidget> CreateCheckBox();
    virtual SharedPtr<GenericWidget> CreateHierarchyList();
};

}

#define URHO3D_IMPLEMENT_WIDGET_FACTORY(factory, implementation) \
    SharedPtr<GenericWidget> factory() override { return MakeShared<implementation>(*this); }

