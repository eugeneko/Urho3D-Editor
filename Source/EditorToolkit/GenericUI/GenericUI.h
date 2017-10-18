#pragma once

#include "KeyBinding.h"
#include "AbstractInput.h"
#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/HashSet.h>
#include <Urho3D/Input/Input.h>

namespace Urho3D
{

class Scene;
class Node;
class Camera;
class Serializable;

class AbstractMainWindow;
class Selection;
class AbstractDock;

/// Abstract UI widget clicked.
URHO3D_EVENT(E_ABSTRACTWIDGETCLICKED, AbstractWidgetClicked)
{
    URHO3D_PARAM(P_ELEMENT, Element);   // AbstractWidget ptr
    URHO3D_PARAM(P_ITEM, Item);         // AbstractWidget ptr (optional)
}

enum class DockLocation
{
    Left,
    Right,
    Top,
    Bottom,
};

struct AbstractAction
{
    String id_;
    String text_;
    std::function<void()> actionCallback_;
    KeyBinding keyBinding_;
};

class AbstractMenu
{
public:
    virtual AbstractMenu* AddMenu(const String& name) = 0;
    virtual AbstractMenu* AddAction(const String& name, const String& actionId) = 0;
};

// #TODO: Rename file
class AbstractWidget : public Object
{
    URHO3D_OBJECT(AbstractWidget, Object);

public:
    AbstractWidget(AbstractMainWindow& mainWindow);

    void SetParent(AbstractWidget* parent);
    AbstractWidget* GetParent() const { return parent_; }

    template <class T> void SetInternalHandle(T pointer) { internalPointer_ = MakeCustomValue(pointer); }
    template <class T> T GetInternalHandle() const { return internalPointer_.GetCustom<T>(); }

    AbstractMainWindow* GetMainWindow() const { return &mainWindow_; }

private:
    /// Called when widget is attached to the root.
    virtual void OnParentSet() { }

protected:
    AbstractMainWindow& mainWindow_;

private:
    AbstractWidget* parent_ = nullptr;
    Variant internalPointer_;
    bool attachedToRoot_ = false;
};

class AbstractDock : public AbstractWidget
{
    URHO3D_OBJECT(AbstractDock, AbstractWidget);

public:
    AbstractDock(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }

    AbstractWidget* CreateContent(StringHash type);
    template <class T> T* CreateContent() { return dynamic_cast<T*>(CreateContent(T::GetTypeStatic())); }

    virtual void SetName(const String& name) = 0;

private:
    bool SetContent(AbstractWidget* content);

    virtual bool DoSetContent(AbstractWidget* content) = 0;

private:
    SharedPtr<AbstractWidget> content_;

};

class AbstractDummyWidget : public AbstractWidget
{
    URHO3D_OBJECT(AbstractDummyWidget, AbstractWidget);

public:
    AbstractDummyWidget(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }
};

class AbstractScrollArea : public AbstractWidget
{
    URHO3D_OBJECT(AbstractScrollArea, AbstractWidget);

public:
    AbstractScrollArea(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }

    virtual void SetDynamicWidth(bool dynamicWidth) = 0;

    AbstractWidget* CreateContent(StringHash type);
    template <class T> T* CreateContent() { return dynamic_cast<T*>(CreateContent(T::GetTypeStatic())); }

private:
    bool SetContent(AbstractWidget* content);

    virtual bool DoSetContent(AbstractWidget* content) = 0;

private:
    SharedPtr<AbstractWidget> content_;

};

class AbstractLayout : public AbstractWidget
{
    URHO3D_OBJECT(AbstractLayout, AbstractWidget);

public:
    AbstractLayout(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }

    AbstractWidget* CreateCell(StringHash type, unsigned row, unsigned column);
    template <class T> T* CreateCell(unsigned row, unsigned column) { return dynamic_cast<T*>(CreateCell(T::GetTypeStatic(), row, column)); }

    AbstractWidget* CreateRow(StringHash type, unsigned row);
    template <class T> T* CreateRow(unsigned row) { return dynamic_cast<T*>(CreateRow(T::GetTypeStatic(), row)); }

    void RemoveRow(unsigned row);
    void RemoveAllChildren();

private:
    bool SetCell(AbstractWidget* cell, unsigned row, unsigned column);
    bool SetRow(AbstractWidget* cell, unsigned row);

    virtual bool DoSetCell(unsigned row, unsigned column, AbstractWidget* child) = 0;
    virtual bool DoSetRow(unsigned row, AbstractWidget* child) = 0;
    virtual void DoRemoveChild(AbstractWidget* child) = 0;

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
        Vector<SharedPtr<AbstractWidget>> columns_;
    };

    bool EnsureRow(unsigned row, RowType type);
    bool EnsureCell(unsigned row, unsigned column, RowType type);

    Vector<RowData> rows_;

};

class AbstractCollapsiblePanel : public AbstractWidget
{
    URHO3D_OBJECT(AbstractCollapsiblePanel, AbstractWidget);

public:
    AbstractCollapsiblePanel(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }

    virtual void SetHeaderText(const String& text) = 0;
    virtual void SetExpanded(bool expanded) = 0;

    AbstractWidget* CreateHeaderPrefix(StringHash type);
    template <class T> T* CreateHeaderPrefix() { return dynamic_cast<T*>(CreateHeaderPrefix(T::GetTypeStatic())); }

    AbstractWidget* CreateHeaderSuffix(StringHash type);
    template <class T> T* CreateHeaderSuffix() { return dynamic_cast<T*>(CreateHeaderSuffix(T::GetTypeStatic())); }

    AbstractWidget* CreateBody(StringHash type);
    template <class T> T* CreateBody() { return dynamic_cast<T*>(CreateBody(T::GetTypeStatic())); }

private:
    virtual bool SetHeaderPrefix(AbstractWidget* header);
    virtual bool SetHeaderSuffix(AbstractWidget* header);
    virtual bool SetBody(AbstractWidget* body);

    virtual bool DoSetHeaderPrefix(AbstractWidget* header) = 0;
    virtual bool DoSetHeaderSuffix(AbstractWidget* header) = 0;
    virtual bool DoSetBody(AbstractWidget* body) = 0;

private:
    SharedPtr<AbstractWidget> headerPrefix_;
    SharedPtr<AbstractWidget> headerSuffix_;
    SharedPtr<AbstractWidget> body_;

};

class AbstractButton : public AbstractWidget
{
    URHO3D_OBJECT(AbstractButton, AbstractWidget);

public:
    AbstractButton(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }
    virtual void SetText(const String& text) = 0;
};

class AbstractText : public AbstractWidget
{
    URHO3D_OBJECT(AbstractText, AbstractWidget);

public:
    AbstractText(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }
    virtual void SetText(const String& text) = 0;
    virtual unsigned GetTextWidth() const = 0;
};

class AbstractLineEdit : public AbstractWidget
{
    URHO3D_OBJECT(AbstractLineEdit, AbstractWidget);

public:
    AbstractLineEdit(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }
    virtual void SetText(const String& text) = 0;
    virtual const String& GetText() const = 0;

public:
    std::function<void()> onTextEdited_;
    std::function<void()> onTextFinished_;
};

class AbstractCheckBox : public AbstractWidget
{
    URHO3D_OBJECT(AbstractCheckBox, AbstractWidget);

public:
    AbstractCheckBox(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }
    virtual void SetChecked(bool checked) = 0;
};

class AbstractHierarchyListItem : public Object
{
    URHO3D_OBJECT(AbstractHierarchyListItem, Object);

public:
    AbstractHierarchyListItem(Context* context) : Object(context) { }
    void SetParent(AbstractHierarchyListItem* parent) { parent_ = parent; }
    void SetInternalPointer(Object* internalPointer) { internalPointer_ = internalPointer; }
    Object* GetInternalPointer() const { return internalPointer_; }

    void InsertChild(AbstractHierarchyListItem* item, unsigned index);
    void RemoveChild(unsigned index);
    AbstractHierarchyListItem* GetParent() const { return parent_; }
    unsigned GetNumChildren() const { return children_.Size(); }
    AbstractHierarchyListItem* GetChild(unsigned index) const { return index < children_.Size() ? children_[index] : nullptr; }
    int GetIndex();

    virtual String GetText() { return String::EMPTY; }

private:
    AbstractHierarchyListItem* parent_ = nullptr;
    Object* internalPointer_ = nullptr;
    Vector<SharedPtr<AbstractHierarchyListItem>> children_;
};

class AbstractHierarchyList : public AbstractWidget
{
    URHO3D_OBJECT(AbstractHierarchyList, AbstractWidget);

public:
    using ItemVector = PODVector<AbstractHierarchyListItem*>;
    AbstractHierarchyList(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }
    virtual void AddItem(AbstractHierarchyListItem* item, unsigned index, AbstractHierarchyListItem* parent) = 0;
    virtual void SelectItem(AbstractHierarchyListItem* item) = 0;
    virtual void DeselectItem(AbstractHierarchyListItem* item) = 0;
    virtual void GetSelection(ItemVector& result) = 0;
    ItemVector GetSelection() { ItemVector result; GetSelection(result); return result; }
};

class AbstractView3D : public AbstractWidget
{
    URHO3D_OBJECT(AbstractView3D, AbstractWidget);

public:
    AbstractView3D(AbstractMainWindow& mainWindow) : AbstractWidget(mainWindow) { }
    /// Set the content of the view.
    virtual void SetView(Scene* scene, Camera* camera) = 0;
    /// Set auto update.
    virtual void SetAutoUpdate(bool autoUpdate) = 0;
    /// Update view.
    virtual void UpdateView() = 0;

};

class AbstractMainWindow
{
public:
    SharedPtr<AbstractWidget> CreateWidget(StringHash type);
    virtual AbstractDock* AddDock(DockLocation hint = DockLocation::Left) = 0;
    virtual void AddAction(const AbstractAction& actionDesc) = 0;
    virtual AbstractMenu* AddMenu(const String& name) = 0;

    virtual Context* GetContext() = 0;
    virtual AbstractInput* GetInput() = 0;

    template <class T> void AddAction(const String& id, KeyBinding keyBinding, T function)
    {
        AddAction({ id, "", function, keyBinding });
    }

private:
    virtual SharedPtr<AbstractWidget> CreateDummyWidget() { return nullptr; }
    virtual SharedPtr<AbstractWidget> CreateScrollArea() { return nullptr; }
    virtual SharedPtr<AbstractWidget> CreateLayout() { return nullptr; }
    virtual SharedPtr<AbstractWidget> CreateCollapsiblePanel() { return nullptr; }
    virtual SharedPtr<AbstractWidget> CreateButton() { return nullptr; }
    virtual SharedPtr<AbstractWidget> CreateText() { return nullptr; }
    virtual SharedPtr<AbstractWidget> CreateLineEdit() { return nullptr; }
    virtual SharedPtr<AbstractWidget> CreateCheckBox() { return nullptr; }
    virtual SharedPtr<AbstractWidget> CreateHierarchyList() { return nullptr; }
    virtual SharedPtr<AbstractWidget> CreateView3D() { return nullptr; }
};

}

#define URHO3D_IMPLEMENT_WIDGET_FACTORY(factory, implementation) \
    SharedPtr<AbstractWidget> factory() override { return MakeShared<implementation>(*this); }

