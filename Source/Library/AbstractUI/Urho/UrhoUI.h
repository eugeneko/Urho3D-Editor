#pragma once

#include "../AbstractUI.h"
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Menu.h>

namespace Urho3D
{

class UI;
class LineEdit;
class Button;
class CheckBox;
class ScrollView;
class View3D;
class ListView;
class DropDownList;

class UrhoMainWindow;
class GridLayout;

class UrhoDock : public AbstractDock
{
    URHO3D_OBJECT(UrhoDock, AbstractDock);

public:
    UrhoDock(AbstractMainWindow* mainWindow);
    void SetSizeHint(const IntVector2& sizeHint);

    void SetName(const String& name) override;


private:
    void OnParentSet() override;
    bool DoSetContent(AbstractWidget* content) override;

private:
    Window* window_ = nullptr;
    Text* windowTitle_ = nullptr;
    Button* buttonClose_ = nullptr;
    UIElement* bodyElement_ = nullptr; // #TODO Remove it
    IntVector2 sizeHint_;
    unsigned layoutNestingLevel_ = 0;
};

class UrhoScrollArea : public AbstractScrollArea
{
    URHO3D_OBJECT(UrhoScrollArea, AbstractScrollArea);

public:
    UrhoScrollArea(AbstractMainWindow* mainWindow);

    void SetDynamicWidth(bool dynamicWidth) override;

private:
    void OnParentSet() override;
    bool DoSetContent(AbstractWidget* content) override;

    void HandleResized(StringHash eventType, VariantMap& eventData);
    void UpdateContentSize();

private:
    ScrollView* scrollView_ = nullptr;
    UIElement* scrollPanel_ = nullptr;
    bool dynamicWidth_ = false;

    unsigned layoutNestingLevel_ = 0;
};

class UrhoLayout : public AbstractLayout
{
    URHO3D_OBJECT(UrhoLayout, AbstractLayout);

public:
    UrhoLayout(AbstractMainWindow* mainWindow);

private:
    void OnParentSet() override;
    bool DoSetCell(unsigned row, unsigned column, AbstractWidget* child) override;
    bool DoSetRow(unsigned row, AbstractWidget* child) override;
    void DoRemoveChild(AbstractWidget* child) override;

private:
    GridLayout* body_ = nullptr;
};

class UrhoCollapsiblePanel : public AbstractCollapsiblePanel
{
    URHO3D_OBJECT(UrhoCollapsiblePanel, AbstractCollapsiblePanel);

public:
    UrhoCollapsiblePanel(AbstractMainWindow* mainWindow);

    void SetHeaderText(const String& text) override;
    void SetExpanded(bool expanded) override;

private:
    void OnParentSet() override;
    bool DoSetHeaderPrefix(AbstractWidget* header) override;
    bool DoSetHeaderSuffix(AbstractWidget* header) override;
    bool DoSetBody(AbstractWidget* body) override;

    void UpdateContentSize();
    void HandleBodyResized(StringHash eventType, VariantMap& eventData);

private:
    BorderImage* panel_ = nullptr;
    UIElement* header_ = nullptr;

    CheckBox* toggleButton_ = nullptr;
    UIElement* headerPrefix_ = nullptr;
    Text* headerText_ = nullptr;
    UIElement* headerSuffix_ = nullptr;

    UIElement* body_ = nullptr;
};

class UrhoWidgetStack : public AbstractWidgetStackT<UIElement>
{
    URHO3D_OBJECT(UrhoWidgetStack, AbstractWidgetStack);

public:
    UrhoWidgetStack(AbstractMainWindow* mainWindow);

private:
    void DoAddChild(UIElement* child) override;
    void DoRemoveChild(UIElement* child) override;
    void DoSelectChild(UIElement* child) override;

    void OnParentSet() override;

private:
    UIElement* element_ = nullptr;

};

class UrhoButton : public AbstractButton
{
    URHO3D_OBJECT(UrhoButton, AbstractButton);

public:
    UrhoButton(AbstractMainWindow* mainWindow);
    void SetText(const String& text) override;

private:
    void OnParentSet() override;

    void UpdateButtonSize();

private:
    Button* button_ = nullptr;
    Text* text_ = nullptr;
};

class UrhoText : public AbstractText
{
    URHO3D_OBJECT(UrhoText, AbstractText);

public:
    UrhoText(AbstractMainWindow* mainWindow);
    void SetText(const String& text) override;
    unsigned GetTextWidth() const override;

private:
    void OnParentSet() override;

private:
    Text* text_ = nullptr;
};

class UrhoLineEdit : public AbstractLineEdit
{
    URHO3D_OBJECT(UrhoLineEdit, AbstractLineEdit);

public:
    UrhoLineEdit(AbstractMainWindow* mainWindow);
    void SetText(const String& text) override;
    String GetText() const override;

private:
    void OnParentSet() override;

private:
    LineEdit* lineEdit_ = nullptr;
    bool suppressTextChange_ = false;
};

class UrhoCheckBox : public AbstractCheckBox
{
    URHO3D_OBJECT(UrhoCheckBox, AbstractCheckBox);

public:
    UrhoCheckBox(AbstractMainWindow* mainWindow);
    void SetChecked(bool checked) override;
    //AbstractCheckBox& SetText(const String& text) override;

private:
    void OnParentSet() override;

private:
    UIElement* panel_ = nullptr;
    CheckBox* checkBox_ = nullptr;
    Text* text_ = nullptr;
};

class UrhoHierarchyList : public AbstractHierarchyList
{
    URHO3D_OBJECT(UrhoHierarchyList, AbstractHierarchyList);

public:
    UrhoHierarchyList(AbstractMainWindow* mainWindow);
    void SetMultiselect(bool multiselect) override;
    void AddItem(AbstractHierarchyListItem* item, unsigned index, AbstractHierarchyListItem* parent) override;
    void RemoveItem(AbstractHierarchyListItem* item) override;
    void RemoveAllItems() override;
    void SelectItem(AbstractHierarchyListItem* item) override;
    void DeselectItem(AbstractHierarchyListItem* item) override;
    void ExpandItem(AbstractHierarchyListItem* item) override;
    void GetSelection(ItemVector& result) override;


private:
    void OnParentSet() override;

    void InsertItem(AbstractHierarchyListItem* item, unsigned index, AbstractHierarchyListItem* parent);

private:
    ListView* hierarchyList_ = nullptr;
    AbstractHierarchyListItem rootItem_;
};

class UrhoView3D : public AbstractView3D
{
    URHO3D_OBJECT(UrhoView3D, AbstractView3D);

public:
    UrhoView3D(AbstractMainWindow* mainWindow);
    void SetView(Scene* scene, Camera* camera) override;
    void SetAutoUpdate(bool autoUpdate) override;
    void UpdateView() override;

private:
    void OnParentSet() override;

private:
    View3D* view3D_ = nullptr;
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

class UrhoMenu : public AbstractMenu
{
    URHO3D_OBJECT(UrhoMenu, AbstractMenu);

public:
    UrhoMenu(UrhoMainWindow* mainWindow, UIElement* parent, const String& text, const String& actionId, bool hasPopup, bool topLevel);
    AbstractMenu* AddMenu(const String& name) override;
    AbstractMenu* AddAction(const String& name, const String& actionId) override;
    void SetName(const String& name) override;

    void OnShowPopup();

private:
    void HandleMenuSelected(StringHash eventType, VariantMap& eventData);

private:
    UrhoMainWindow* mainWindow_ = nullptr;
    // #TODO Hide em
    Menu* menu_ = nullptr;
    Text* text_ = nullptr;
    SharedPtr<Window> popup_ = nullptr;
    std::function<void()> actionCallback_;
    Vector<SharedPtr<UrhoMenu>> children_;
};

class MenuWithPopupCallback : public Menu
{
public:
    MenuWithPopupCallback(Context* context, UrhoMenu* menu);

    virtual void OnShowPopup() override;

private:
    UrhoMenu* menu_ = nullptr;
};

class UrhoMainWindow : public AbstractMainWindow, public Object
{
    URHO3D_OBJECT(UrhoMainWindow, Object);

public:
    static const StringHash VAR_DOCUMENT;

    UrhoMainWindow(Context* context);

    AbstractDock* AddDock(DockLocation hint, const IntVector2& sizeHint) override;
    void AddAction(const AbstractAction& actionDesc) override;
    AbstractMenu* AddMenu(const String& name) override;
    void InsertDocument(Object* document, const String& title, unsigned index) override;
    void SelectDocument(Object* document) override;
    PODVector<Object*> GetDocuments() const override;

    Context* GetContext() override { return Object::GetContext(); }
    AbstractInput* GetInput() override { return &input_; }

    AbstractAction* FindAction(const String& actionId) const;
    void CollapseMenuPopups(Menu* menu) const;

private:
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateScrollArea,       UrhoScrollArea);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateLayout,           UrhoLayout);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateCollapsiblePanel, UrhoCollapsiblePanel);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateWidgetStack,      UrhoWidgetStack);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateButton,           UrhoButton);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateText,             UrhoText);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateLineEdit,         UrhoLineEdit);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateCheckBox,         UrhoCheckBox);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateHierarchyList,    UrhoHierarchyList);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateView3D,           UrhoView3D);

private:
    UI* ui_ = nullptr;
    UIElement* uiRoot_ = nullptr;
    DockStation* mainElement_ = nullptr;

    Vector<SharedPtr<UrhoDock>> dialogs_;
    StandardUrhoInput input_;

    HashMap<String, AbstractAction> actions_;

    UIElement* menuBar_ = nullptr;
    Vector<SharedPtr<UrhoMenu>> menus_;

    UIElement* documentBar_ = nullptr;
    DropDownList* documentList_ = nullptr;
    HashSet<SharedPtr<Object>> documents_;
};

}
