#pragma once

#include "GenericUI.h"
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/Text.h>

namespace Urho3D
{

class UrhoMainWindow;
class UI;
class Menu;
class LineEdit;
class Button;
class ScrollView;

class UrhoWidget
{
public:
    virtual UIElement* CreateElement(UIElement* parent) = 0;
};

class UrhoDialog : public GenericDialog
{
    URHO3D_OBJECT(UrhoDialog, GenericDialog);

public:
    UrhoDialog(AbstractMainWindow& mainWindow, GenericWidget* parent);
    void SetName(const String& name) override;

private:
    bool SetContent(GenericWidget* content) override;

private:
    SharedPtr<Window> window_;
    Text* windowTitle_ = nullptr;
    UIElement* bodyElement_ = nullptr; // #TODO Remove it
};

class UrhoScrollArea : public AbstractScrollArea, public UrhoWidget
{
    URHO3D_OBJECT(UrhoScrollArea, AbstractScrollArea);

public:
    UrhoScrollArea(AbstractMainWindow& mainWindow, GenericWidget* parent);

    void SetDynamicWidth(bool dynamicWidth) override;

    UIElement* CreateElement(UIElement* parent) override;

private:
    bool SetContent(GenericWidget* content) override;

    void HandleResized(StringHash eventType, VariantMap& eventData);
    void UpdateContentSize();

private:
    ScrollView* scrollView_ = nullptr;
    UIElement* scrollPanel_ = nullptr;
    bool dynamicWidth_ = false;
};

class UrhoLayout : public AbstractLayout, public UrhoWidget
{
    URHO3D_OBJECT(UrhoLayout, AbstractLayout);

public:
    UrhoLayout(AbstractMainWindow& mainWindow, GenericWidget* parent);

    void UpdateLayout();

    UIElement* CreateElement(UIElement* parent) override;

private:
    bool SetCellWidget(unsigned row, unsigned column, GenericWidget* child) override;
    bool SetRowWidget(unsigned row, GenericWidget* child) override;

    void HandleLayoutChanged(StringHash eventType, VariantMap& eventData);
    enum class RowType
    {
        SingleColumn,
        MultipleColumns
    };

private:
    UIElement* body_ = nullptr;

    Vector<Pair<Vector<UIElement*>, RowType>> elements_;
};

class UrhoButton : public AbstractButton, public UrhoWidget
{
    URHO3D_OBJECT(UrhoButton, AbstractButton);

public:
    UrhoButton(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractButton(mainWindow, parent) {}
    AbstractButton& SetText(const String& text) override;

private:
    UIElement* CreateElement(UIElement* parent) override;

    void UpdateContainerSize();

private:
    Button* button_ = nullptr;
    Text* text_ = nullptr;
};

class UrhoText : public AbstractText, public UrhoWidget
{
    URHO3D_OBJECT(UrhoText, AbstractText);

public:
    UrhoText(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractText(mainWindow, parent) {}
    AbstractText& SetText(const String& text) override;

private:
    UIElement* CreateElement(UIElement* parent) override;

private:
    UIElement* container_ = nullptr;
    Text* text_ = nullptr;
};

class UrhoLineEdit : public AbstractLineEdit, public UrhoWidget
{
    URHO3D_OBJECT(UrhoLineEdit, AbstractLineEdit);

public:
    UrhoLineEdit(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractLineEdit(mainWindow, parent) {}
    AbstractLineEdit& SetText(const String& text) override;

private:
    UIElement* CreateElement(UIElement* parent) override;

private:
    LineEdit* lineEdit_ = nullptr;
};

class UrhoHierarchyListItemWidget : public Text
{
public:
    UrhoHierarchyListItemWidget(Context* context, GenericHierarchyListItem* item);
    GenericHierarchyListItem* GetItem() { return item_; }
private:
    GenericHierarchyListItem* item_ = nullptr;;
};

class UrhoHierarchyList : public GenericHierarchyList, public UrhoWidget
{
    URHO3D_OBJECT(UrhoHierarchyList, GenericHierarchyList);

public:
    UrhoHierarchyList(AbstractMainWindow& mainWindow, GenericWidget* parent);
    void AddItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent) override;
    void SelectItem(GenericHierarchyListItem* item) override;
    void DeselectItem(GenericHierarchyListItem* item) override;
    void GetSelection(ItemVector& result) override;

    UIElement* CreateElement(UIElement* parent) override;

private:
    void InsertItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent);
    void HandleItemClicked(StringHash eventType, VariantMap& eventData);

private:
    SharedPtr<ListView> hierarchyList_;
    GenericHierarchyListItem rootItem_;
};

class StandardUrhoInput : public StandardInput, public Object
{
    URHO3D_OBJECT(StandardUrhoInput, Object);

public:
    /// Construct.
    StandardUrhoInput(Context* context);

    /// \see AbstractInput::SetMouseMode
    void SetMouseMode(MouseMode mouseMode) override;
    /// \see AbstractInput::IsUIFocused
    bool IsUIFocused() const override;
    /// \see AbstractInput::IsUIHovered
    bool IsUIHovered() const override;

    /// \see AbstractInput::IsUIHovered
    bool IsKeyDown(int key) const override;
    /// \see AbstractInput::IsUIHovered
    bool IsKeyPressed(int key) const override;
    /// \see AbstractInput::IsUIHovered
    bool IsMouseButtonDown(int mouseButton) const override;
    /// \see AbstractInput::IsUIHovered
    bool IsMouseButtonPressed(int mouseButton) const override;
    /// \see AbstractInput::IsUIHovered
    IntVector2 GetMousePosition() const override;
    /// \see AbstractInput::IsUIHovered
    IntVector2 GetMouseMove() const override;
    /// \see AbstractInput::IsUIHovered
    int GetMouseWheelMove() const override;

private:
    /// Input.
    Input* input_ = nullptr;
    /// UI.
    UI* ui_ = nullptr;
};

class UrhoMenu : public GenericMenu, public Object
{
    URHO3D_OBJECT(UrhoMenu, Object);

public:
    static const StringHash VAR_ACTION;
    UrhoMenu(UrhoMainWindow& mainWindow, UIElement* parent, const String& text, const String& actionId, bool hasPopup, bool topLevel);
    GenericMenu* AddMenu(const String& name) override;
    GenericMenu* AddAction(const String& name, const String& actionId) override;

private:
    void HandleMenuSelected(StringHash eventType, VariantMap& eventData);

private:
    UrhoMainWindow& mainWindow_;
    // #TODO Hide em
    Menu* menu_ = nullptr;
    Text* text_ = nullptr;
    SharedPtr<Window> popup_ = nullptr;
    std::function<void()> actionCallback_;
    Vector<SharedPtr<UrhoMenu>> children_;
};

class UrhoMainWindow : public AbstractMainWindow, public Object
{
    URHO3D_OBJECT(UrhoMainWindow, Object);

public:
    UrhoMainWindow(Context* context);

    GenericDialog* AddDialog(DialogLocationHint hint = DialogLocationHint::Undocked) override;
    void AddAction(const AbstractAction& actionDesc) override;
    GenericMenu* AddMenu(const String& name) override;

    Context* GetContext() override { return Object::GetContext(); }
    AbstractInput* GetInput() override { return &input_; }

    AbstractAction* FindAction(const String& actionId) const;
    void CollapseMenuPopups(Menu* menu) const;

private:
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateScrollArea,       UrhoScrollArea);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateLayout,           UrhoLayout);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateButton,           UrhoButton);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateText,             UrhoText);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateLineEdit,         UrhoLineEdit);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateHierarchyList,    UrhoHierarchyList);

    void HandleResized(StringHash eventType, VariantMap& eventData);

private:
    Vector<SharedPtr<UrhoDialog>> dialogs_;
    StandardUrhoInput input_;

    HashMap<String, AbstractAction> actions_;

    UIElement* menuBar_ = nullptr;
    Vector<SharedPtr<UrhoMenu>> menus_;
};

}
