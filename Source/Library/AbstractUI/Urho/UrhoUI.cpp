#include "UrhoUI.h"
#include "GridLayout.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/DropDownList.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/View3D.h>
#include <Urho3D/UI/LineEdit.h>
#include <numeric>

namespace Urho3D
{

namespace
{

String PrintKey(int key)
{
    switch (key)
    {
    case KEY_BACKSPACE: return "Backspace";
    case KEY_TAB: return "Tab";
    case KEY_RETURN: return "Return";
    case KEY_RETURN2: return "Return2";
    case KEY_KP_ENTER: return "NumEnter";
    case KEY_SHIFT: return "Shift";
    case KEY_CTRL: return "Ctrl";
    case KEY_ALT: return "Alt";
    case KEY_GUI: return "GUI";
    case KEY_PAUSE: return "Pause";
    case KEY_CAPSLOCK: return "CapsLock";
    case KEY_ESCAPE: return "Esc";
    case KEY_SPACE: return "Space";
    case KEY_PAGEUP: return "PageUp";
    case KEY_PAGEDOWN: return "PageDn";
    case KEY_END: return "End";
    case KEY_HOME: return "Home";
    case KEY_LEFT: return "Left";
    case KEY_UP: return "Up";
    case KEY_RIGHT: return "Right";
    case KEY_DOWN: return "Down";
    case KEY_SELECT: return "Select";
    case KEY_PRINTSCREEN: return "PrintScr";
    case KEY_INSERT: return "Ins";
    case KEY_DELETE: return "Del";
    case KEY_APPLICATION: return "App";
    case KEY_KP_0: return "Num0";
    case KEY_KP_1: return "Num1";
    case KEY_KP_2: return "Num2";
    case KEY_KP_3: return "Num3";
    case KEY_KP_4: return "Num4";
    case KEY_KP_5: return "Num5";
    case KEY_KP_6: return "Num6";
    case KEY_KP_7: return "Num7";
    case KEY_KP_8: return "Num8";
    case KEY_KP_9: return "Num9";
    case KEY_KP_MULTIPLY: return "NumMul";
    case KEY_KP_PLUS: return "NumPlus";
    case KEY_KP_MINUS: return "NumMinus";
    case KEY_KP_PERIOD: return "NumPeriod";
    case KEY_KP_DIVIDE: return "NumDiv";
    case KEY_F1: return "F1";
    case KEY_F2: return "F2";
    case KEY_F3: return "F3";
    case KEY_F4: return "F4";
    case KEY_F5: return "F5";
    case KEY_F6: return "F6";
    case KEY_F7: return "F7";
    case KEY_F8: return "F8";
    case KEY_F9: return "F9";
    case KEY_F10: return "F10";
    case KEY_F11: return "F11";
    case KEY_F12: return "F12";
    case KEY_NUMLOCKCLEAR: return "NumLock";
    case KEY_SCROLLLOCK: return "ScrollLock";
    default:
        if (key >= KEY_0 && key <= KEY_9)
            return String(static_cast<char>(key - KEY_0 + '0'));
        else if (key >= KEY_A && key <= KEY_Z)
            return String(static_cast<char>(key - KEY_A + 'A'));
        else
            return "Unknown";
    }
}

String PrintKeyBinding(const KeyBinding& keyBinding)
{
    String result;
    if (keyBinding.GetShift() == ModifierState::Required)
        result += "Shift+";
    if (keyBinding.GetCtrl() == ModifierState::Required)
        result += "Ctrl+";
    if (keyBinding.GetAlt() == ModifierState::Required)
        result += "Alt+";
    result += PrintKey(keyBinding.GetKey());
    return result;
}

UIElement* GetInternalElement(AbstractWidget* widget)
{
    return widget ? widget->GetInternalHandle<UIElement*>() : nullptr;
}

void SetInternalElement(AbstractWidget* widget, UIElement* element)
{
    widget->SetInternalHandle(element);
}

UIElement* GetInternalElement(AbstractHierarchyListItem* item)
{
    return item ? item->GetInternalHandle<UIElement*>() : nullptr;
}

void SetInternalElement(AbstractHierarchyListItem* item, UIElement* element)
{
    item->SetInternalHandle(element);
}

UIElement* GetParentElement(AbstractWidget* widget)
{
    return GetInternalElement(widget->GetParent());
}

static const StringHash elementItem("Item");

void SetElementItem(UIElement* element, AbstractHierarchyListItem* widget)
{
    element->SetVar(elementItem, widget);
}

AbstractHierarchyListItem* GetElementItem(UIElement* element)
{
    return static_cast<AbstractHierarchyListItem*>(element->GetVar(elementItem).GetPtr());
}

}

//////////////////////////////////////////////////////////////////////////
UrhoDock::UrhoDock(AbstractMainWindow* mainWindow)
    : AbstractDock(mainWindow)
    , window_(new Window(context_))
{
    sizeHint_ = IntVector2(200, 200);
    SetInternalElement(this, window_);
}

void UrhoDock::SetSizeHint(const IntVector2& sizeHint)
{
    sizeHint_ = sizeHint;
}

void UrhoDock::SetName(const String& name)
{
    window_->SetName(name);
    windowTitle_->SetText(name);
}

void UrhoDock::OnParentSet()
{
    window_->DisableLayoutUpdate();

    // Create window
    window_->SetStyleAuto();
    window_->SetPosition(200, 200);
    window_->SetSize(sizeHint_);
    window_->SetResizeBorder(IntRect(6, 6, 6, 6));
    window_->SetResizable(true);
    window_->SetMovable(true);
    window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));

    // Create title
    UIElement* titleBar = window_->CreateChild<UIElement>();
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);
    titleBar->SetFixedHeight(titleBar->GetMinHeight());

    windowTitle_ = titleBar->CreateChild<Text>("AD_WindowTitle");
    windowTitle_->SetStyleAuto();

    buttonClose_ = titleBar->CreateChild<Button>("AD_CloseButton");
    buttonClose_->SetStyle("CloseButton");

    bodyElement_ = window_->CreateChild<UIElement>();
    bodyElement_->SetLayoutMode(LM_VERTICAL);
    bodyElement_->CreateChild<UIElement>(); // Create stub element to avoid collapsing bodyElement_

    window_->EnableLayoutUpdate();
    window_->UpdateLayout();
}

bool UrhoDock::DoSetContent(AbstractWidget* content)
{
    if (!GetInternalElement(content))
        return false;

    bodyElement_->RemoveAllChildren();
    bodyElement_->AddChild(GetInternalElement(content));
    return true;
}

//////////////////////////////////////////////////////////////////////////
UrhoScrollArea::UrhoScrollArea(AbstractMainWindow* mainWindow)
    : AbstractScrollArea(mainWindow)
    , scrollView_(new ScrollView(context_))
{
    scrollView_->SetName("ASR_ScrollView");
    SetInternalElement(this, scrollView_);
}

void UrhoScrollArea::SetDynamicWidth(bool dynamicWidth)
{
    UIElement* body = scrollView_->GetContentElement();
    if (dynamicWidth)
    {
        dynamicWidth_ = true;
        UpdateContentSize();
    }
    else
    {
        dynamicWidth_ = false;
        if (body)
        {
            body->SetMinWidth(0);
            body->SetMaxWidth(M_MAX_INT);
        }
    }
}

bool UrhoScrollArea::DoSetContent(AbstractWidget* content)
{
    if (!GetInternalElement(content))
        return false;

    scrollView_->SetContentElement(GetInternalElement(content));
    return true;
}

void UrhoScrollArea::OnParentSet()
{
    scrollView_->SetStyleAuto();

    scrollPanel_ = scrollView_->GetScrollPanel();
    SubscribeToEvent(scrollPanel_, E_LAYOUTUPDATED, URHO3D_HANDLER(UrhoScrollArea, HandleResized));
}

void UrhoScrollArea::HandleResized(StringHash /*eventType*/, VariantMap& /*eventData*/)
{
    UpdateContentSize();
}

void UrhoScrollArea::UpdateContentSize()
{
    if (layoutNestingLevel_ > 0)
        return;

    ++layoutNestingLevel_;
    UIElement* body = scrollView_->GetContentElement();
    const IntRect& clipBorder = scrollPanel_->GetClipBorder();
    if (body)
    {
        body->SetWidth(Max(body->GetMinWidth(), scrollPanel_->GetWidth() - clipBorder.left_ - clipBorder.right_));
        body->SetHeight(Max(body->GetMinHeight(), scrollPanel_->GetHeight() - clipBorder.top_ - clipBorder.bottom_));
    }
    --layoutNestingLevel_;
}

//////////////////////////////////////////////////////////////////////////
UrhoLayout::UrhoLayout(AbstractMainWindow* mainWindow)
    : AbstractLayout(mainWindow)
    , body_(new GridLayout(context_))
{
    body_->SetName("AL_GridLayout");
    SetInternalElement(this, body_);
}

void UrhoLayout::OnParentSet()
{
}

bool UrhoLayout::DoSetCell(unsigned row, unsigned column, AbstractWidget* child)
{
    if (!GetInternalElement(child))
        return false;

    body_->InsertItem(row, column, GetInternalElement(child));
    body_->SetRowGroup(row, 0);
    body_->UpdateLayout();
    return true;
}

bool UrhoLayout::DoSetRow(unsigned row, AbstractWidget* child)
{
    if (!GetInternalElement(child))
        return false;

    body_->InsertItem(row, 0, GetInternalElement(child));
    body_->SetRowGroup(row, 1);
    body_->UpdateLayout();
    return true;
}

void UrhoLayout::DoRemoveChild(AbstractWidget* child)
{
    if (UIElement* childElement = GetInternalElement(child))
        body_->RemoveChild(childElement);
    SetInternalElement(child, nullptr);
    body_->UpdateLayout();
}

//////////////////////////////////////////////////////////////////////////
UrhoCollapsiblePanel::UrhoCollapsiblePanel(AbstractMainWindow* mainWindow)
    : AbstractCollapsiblePanel(mainWindow)
    , panel_(new BorderImage(context_))
{
    panel_->SetName("CP_Panel");
    SetInternalElement(this, panel_);
}

void UrhoCollapsiblePanel::SetHeaderText(const String& text)
{
    headerText_->SetText(text);
}

void UrhoCollapsiblePanel::SetExpanded(bool expanded)
{
    toggleButton_->SetChecked(expanded);
    UpdateContentSize();
}

bool UrhoCollapsiblePanel::DoSetHeaderPrefix(AbstractWidget* header)
{
    if (!GetInternalElement(header))
        return false;

    headerPrefix_->RemoveAllChildren();
    headerPrefix_->AddChild(GetInternalElement(header));

    return true;
}

bool UrhoCollapsiblePanel::DoSetHeaderSuffix(AbstractWidget* header)
{
    if (!GetInternalElement(header))
        return false;

    headerSuffix_->RemoveAllChildren();
    headerSuffix_->AddChild(GetInternalElement(header));

    return true;
}

bool UrhoCollapsiblePanel::DoSetBody(AbstractWidget* body)
{
    if (!GetInternalElement(body))
        return false;

    if (body_)
        panel_->RemoveChild(body_);
    body_ = GetInternalElement(body);
    panel_->AddChild(body_);
    SubscribeToEvent(body_, E_RESIZED, URHO3D_HANDLER(UrhoCollapsiblePanel, HandleBodyResized));

    UpdateContentSize();

    return true;
}

void UrhoCollapsiblePanel::UpdateContentSize()
{
    const bool expanded = toggleButton_->IsChecked();
    const int bodyHeight = body_ && expanded ? body_->GetEffectiveMinSize().y_ : 0;
    const int headerHeight = header_->GetEffectiveMinSize().y_;
    if (body_ && !expanded)
        body_->SetVisible(false);
    panel_->SetFixedHeight(bodyHeight + headerHeight);
    if (body_ && expanded)
        body_->SetVisible(true);
}

void UrhoCollapsiblePanel::HandleBodyResized(StringHash eventType, VariantMap& eventData)
{
    UpdateContentSize();
}

void UrhoCollapsiblePanel::OnParentSet()
{
    panel_->SetStyle("ToolTipBorderImage");
    panel_->SetLayout(LM_VERTICAL);

    header_ = panel_->CreateChild<UIElement>("CP_Header");
    header_->SetLayout(LM_HORIZONTAL);

    toggleButton_ = header_->CreateChild<CheckBox>("CP_ToggleButton");
    toggleButton_->SetStyle("HierarchyListViewOverlay");
    SubscribeToEvent(toggleButton_, E_TOGGLED,
        [this](StringHash /*eventType*/, VariantMap& /*eventData*/)
    {
        SetExpanded(toggleButton_->IsChecked());
    });

    headerPrefix_ = header_->CreateChild<UIElement>("CP_HeaderPrefix");
    headerPrefix_->SetLayout(LM_HORIZONTAL);
    headerText_ = header_->CreateChild<Text>("CP_HeaderText");
    headerText_->SetStyleAuto();
    headerSuffix_ = header_->CreateChild<UIElement>("CP_HeaderSuffix");
    headerSuffix_->SetLayout(LM_HORIZONTAL);
}

//////////////////////////////////////////////////////////////////////////
UrhoWidgetStack::UrhoWidgetStack(AbstractMainWindow* mainWindow)
    : AbstractWidgetStackT<UIElement>(mainWindow)
    , element_(new UIElement(context_))
{
    element_->SetName("WS_Contaier");
    SetInternalElement(this, element_);
}

void UrhoWidgetStack::DoAddChild(UIElement* child)
{
    child->SetVisible(false);
    element_->AddChild(child);
}

void UrhoWidgetStack::DoRemoveChild(UIElement* child)
{
    element_->RemoveChild(child);
}

void UrhoWidgetStack::DoSelectChild(UIElement* child)
{
    element_->DisableLayoutUpdate();
    for (UIElement* item : element_->GetChildren())
        item->SetVisible(child == item);
    element_->EnableLayoutUpdate();
    element_->UpdateLayout();
}

void UrhoWidgetStack::OnParentSet()
{
    element_->SetLayout(LM_VERTICAL);
}

//////////////////////////////////////////////////////////////////////////
UrhoButton::UrhoButton(AbstractMainWindow* mainWindow)
    : AbstractButton(mainWindow)
    , button_(new Button(context_))
{
    button_->SetName("AB_Button");
    SetInternalElement(this, button_);
}

void UrhoButton::SetText(const String& text)
{
    text_->SetText(text);
    UpdateButtonSize();
}

void UrhoButton::OnParentSet()
{
    button_->SetStyleAuto();
    button_->SetClipChildren(true);

    text_ = button_->CreateChild<Text>("AB_Text");
    text_->SetStyleAuto();
    text_->SetAlignment(HA_CENTER, VA_CENTER);
    text_->SetMinHeight(static_cast<int>(text_->GetRowHeight()));

    UpdateButtonSize();
}

void UrhoButton::UpdateButtonSize()
{
    IntVector2 size = text_->GetMinSize() + IntVector2(4, 4);
    size.x_ = Max(size.x_, size.y_);
    button_->SetFixedSize(size);
}

//////////////////////////////////////////////////////////////////////////
UrhoText::UrhoText(AbstractMainWindow* mainWindow)
    : AbstractText(mainWindow)
    , text_(new Text(context_))
{
    text_->SetName("AT_Text");
    SetInternalElement(this, text_);
}

void UrhoText::SetText(const String& text)
{
    text_->SetText(text);
    IntVector2 size = text_->GetMinSize();
    size.x_ = Max(size.x_, size.y_);
    text_->SetFixedSize(size);
}

unsigned UrhoText::GetTextWidth() const
{
    return text_->GetMinWidth();
}

void UrhoText::OnParentSet()
{
    text_->SetStyleAuto();
}

//////////////////////////////////////////////////////////////////////////
UrhoLineEdit::UrhoLineEdit(AbstractMainWindow* mainWindow)
    : AbstractLineEdit(mainWindow)
    , lineEdit_(new LineEdit(context_))
{
    lineEdit_->SetName("ALE_LineEdit");
    SetInternalElement(this, lineEdit_);
}

void UrhoLineEdit::SetText(const String& text)
{
    suppressTextChange_ = true;
    lineEdit_->SetText(text);
    suppressTextChange_ = false;
}

String UrhoLineEdit::GetText() const
{
    return lineEdit_->GetText();
}

void UrhoLineEdit::OnParentSet()
{
    lineEdit_->SetStyleAuto();
    const int defaultHeight = static_cast<int>(lineEdit_->GetTextElement()->GetRowHeight());
    lineEdit_->SetMinSize(defaultHeight * 2, defaultHeight);
    SubscribeToEvent(lineEdit_, E_TEXTCHANGED, [this](StringHash /*eventType*/, VariantMap& /*eventData*/)
    {
        if (!suppressTextChange_ && onTextEdited_)
            onTextEdited_();
    });
    SubscribeToEvent(lineEdit_, E_TEXTFINISHED, [this](StringHash /*eventType*/, VariantMap& /*eventData*/)
    {
        if (onTextFinished_)
            onTextFinished_();
    });
}

//////////////////////////////////////////////////////////////////////////
UrhoCheckBox::UrhoCheckBox(AbstractMainWindow* mainWindow)
    : AbstractCheckBox(mainWindow)
    , panel_(new UIElement(context_))
{
    panel_->SetName("ACB_Panel");
    SetInternalElement(this, panel_);
}

void UrhoCheckBox::SetChecked(bool checked)
{
    checkBox_->SetChecked(checked);
}

void UrhoCheckBox::OnParentSet()
{
    panel_->SetLayout(LM_HORIZONTAL);
    checkBox_ = panel_->CreateChild<CheckBox>("ACB_CheckBox");
    checkBox_->SetStyleAuto();
    text_ = panel_->CreateChild<Text>("ACB_Text");
    text_->SetStyleAuto();
}

//////////////////////////////////////////////////////////////////////////
UrhoHierarchyList::UrhoHierarchyList(AbstractMainWindow* mainWindow)
    : AbstractHierarchyList(mainWindow)
    , rootItem_(context_)
    , hierarchyList_(new ListView(context_))
{
    hierarchyList_->SetName("AHL_HierarchyList");
    SetInternalElement(this, hierarchyList_);
}

void UrhoHierarchyList::SetMultiselect(bool multiselect)
{
    hierarchyList_->SetMultiselect(multiselect);
}

void UrhoHierarchyList::AddItem(AbstractHierarchyListItem* item, unsigned index, AbstractHierarchyListItem* parent)
{
    hierarchyList_->DisableInternalLayoutUpdate();
    if (parent)
        parent->InsertChild(item, index);
    else
        rootItem_.InsertChild(item, index);
    InsertItem(item, index, parent);
    hierarchyList_->EnableInternalLayoutUpdate();
    hierarchyList_->UpdateInternalLayout();
}

void UrhoHierarchyList::RemoveItem(AbstractHierarchyListItem* item)
{

}

void UrhoHierarchyList::RemoveAllItems()
{
    hierarchyList_->RemoveAllItems();
}

void UrhoHierarchyList::SelectItem(AbstractHierarchyListItem* item)
{
    if (auto itemWidget = GetInternalElement(item))
    {
        const unsigned index = hierarchyList_->FindItem(itemWidget);
        if (!hierarchyList_->IsSelected(index))
            hierarchyList_->ToggleSelection(index);
    }
}

void UrhoHierarchyList::DeselectItem(AbstractHierarchyListItem* item)
{
    if (auto itemWidget = GetInternalElement(item))
    {
        const unsigned index = hierarchyList_->FindItem(itemWidget);
        if (hierarchyList_->IsSelected(index))
            hierarchyList_->ToggleSelection(index);
    }
}

void UrhoHierarchyList::ExpandItem(AbstractHierarchyListItem* item)
{
    while (item)
    {
        UIElement* element = GetInternalElement(item);
        const unsigned elementIndex = hierarchyList_->FindItem(element);
        hierarchyList_->Expand(elementIndex, true);
        item = item->GetParent();
    }
}

void UrhoHierarchyList::GetSelection(ItemVector& result)
{
    for (unsigned index : hierarchyList_->GetSelections())
    {
        UIElement* element = hierarchyList_->GetItem(index);
        if (auto item = GetElementItem(element))
            result.Push(item);
    }
}

void UrhoHierarchyList::OnParentSet()
{
    hierarchyList_->SetStyle("HierarchyListView");
    hierarchyList_->SetInternal(true);
    hierarchyList_->SetHighlightMode(HM_ALWAYS);
    hierarchyList_->SetSelectOnClickEnd(true);
    hierarchyList_->SetHierarchyMode(true);
    SubscribeToEvent(hierarchyList_, E_ITEMCLICKED,
        [=](StringHash /*eventType*/, VariantMap& eventData)
    {
        UIElement* element = static_cast<UIElement*>(eventData[ItemClicked::P_ITEM].GetPtr());
        if (auto item = GetElementItem(element))
        {
            if (onItemClicked_)
                onItemClicked_(item);
        }
    });
    SubscribeToEvent(hierarchyList_, E_ITEMDOUBLECLICKED,
        [=](StringHash /*eventType*/, VariantMap& eventData)
    {
        UIElement* element = static_cast<UIElement*>(eventData[ItemClicked::P_ITEM].GetPtr());
        if (auto item = GetElementItem(element))
        {
            if (onItemDoubleClicked_)
                onItemDoubleClicked_(item);
        }
    });
}

void UrhoHierarchyList::InsertItem(AbstractHierarchyListItem* item, unsigned index, AbstractHierarchyListItem* parent)
{
    auto itemWidget = MakeShared<Text>(context_);
    SetElementItem(itemWidget, item);
    itemWidget->SetText(item->GetText());
    itemWidget->SetStyle("FileSelectorListText");
    SetInternalElement(item, itemWidget);

    UIElement* parentWidget = GetInternalElement(parent);
    hierarchyList_->InsertItem(M_MAX_UNSIGNED, itemWidget, parentWidget);
    for (unsigned i = 0; i < item->GetNumChildren(); ++i)
        InsertItem(item->GetChild(i), M_MAX_UNSIGNED, item);
    if (parentWidget)
        hierarchyList_->Expand(hierarchyList_->FindItem(itemWidget), false);
}

//////////////////////////////////////////////////////////////////////////
UrhoView3D::UrhoView3D(AbstractMainWindow* mainWindow)
    : AbstractView3D(mainWindow)
    , view3D_(new View3D(context_))
{
    view3D_->SetName("AV_View3D");
    SetInternalElement(this, view3D_);
}

void UrhoView3D::SetView(Scene* scene, Camera* camera)
{
    view3D_->SetView(scene, camera);
}

void UrhoView3D::SetAutoUpdate(bool autoUpdate)
{
    view3D_->SetAutoUpdate(autoUpdate);
}

void UrhoView3D::UpdateView()
{
    view3D_->QueueUpdate();
}

void UrhoView3D::OnParentSet()
{
}

//////////////////////////////////////////////////////////////////////////
StandardUrhoInput::StandardUrhoInput(Context* context)
    : Object(context)
    , input_(GetSubsystem<Input>())
    , ui_(GetSubsystem<UI>())
{

}

void StandardUrhoInput::SetMouseMode(Urho3D::MouseMode mouseMode)
{
    input_->SetMouseMode(mouseMode);
}

bool StandardUrhoInput::IsUIFocused() const
{
    return ui_->HasModalElement() || ui_->GetFocusElement();
}

bool StandardUrhoInput::IsUIHovered() const
{
    return !!ui_->GetElementAt(GetMousePosition());
}

bool StandardUrhoInput::IsKeyDown(int key) const
{
    return input_->GetKeyDown(key);
}

bool StandardUrhoInput::IsKeyPressed(int key) const
{
    return input_->GetKeyPress(key);
}

bool StandardUrhoInput::IsMouseButtonDown(int mouseButton) const
{
    return input_->GetMouseButtonDown(mouseButton);
}

bool StandardUrhoInput::IsMouseButtonPressed(int mouseButton) const
{
    return input_->GetMouseButtonPress(mouseButton);
}

IntVector2 StandardUrhoInput::GetMousePosition() const
{
    return input_->GetMousePosition();
}

IntVector2 StandardUrhoInput::GetMouseMove() const
{
    return input_->GetMouseMove();
}

int StandardUrhoInput::GetMouseWheelMove() const
{
    return input_->GetMouseMoveWheel();
}

//////////////////////////////////////////////////////////////////////////
UrhoMenu::UrhoMenu(UrhoMainWindow* mainWindow, UIElement* parent, const String& text, const String& actionId,
    bool hasPopup, bool topLevel)
    : AbstractMenu(mainWindow->GetContext())
    , mainWindow_(mainWindow)
{
    AbstractAction* action = nullptr;
    if (!actionId.Empty())
    {
        action = mainWindow_->FindAction(actionId);
    }

    menu_ = new MenuWithPopupCallback(context_, this);
    menu_->SetDefaultStyle(parent->GetDefaultStyle());
    menu_->SetStyleAuto();
    menu_->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));

    text_ = menu_->CreateChild<Text>();
    text_->SetStyle("EditorMenuText");
    text_->SetText(text);

    if (topLevel)
    {
        menu_->SetMaxWidth(text_->GetWidth() + 20);
    }

    if (action)
    {
        actionCallback_ = action->actionCallback_;
        SubscribeToEvent(menu_, E_MENUSELECTED, URHO3D_HANDLER(UrhoMenu, HandleMenuSelected));

        const KeyBinding& keyBinding = action->keyBinding_;
        if (!keyBinding.IsEmpty())
        {
            // Setup accelerator
            int qualifiers = 0;
            if (keyBinding.GetShift() == ModifierState::Required)
                qualifiers |= QUAL_SHIFT;
            if (keyBinding.GetCtrl() == ModifierState::Required)
                qualifiers |= QUAL_CTRL;
            if (keyBinding.GetAlt() == ModifierState::Required)
                qualifiers |= QUAL_ALT;
            menu_->SetAccelerator(keyBinding.GetKey(), qualifiers);

            // Create accelerator tip
            UIElement* spacer = menu_->CreateChild<UIElement>();
            spacer->SetMinWidth(text_->GetIndentSpacing());
            spacer->SetHeight(text_->GetHeight());
            menu_->AddChild(spacer);

            Text* accelKeyText = menu_->CreateChild<Text>();
            accelKeyText->SetStyle("EditorMenuText");
            accelKeyText->SetTextAlignment(HA_RIGHT);
            accelKeyText->SetText(PrintKeyBinding(keyBinding));
        }
    }
    else
    {
        SubscribeToEvent(menu_, E_PRESSED,
            [=](StringHash /*eventType*/, VariantMap& /*eventData*/)
        {
            if (onShown_)
                onShown_();
        });
    }

    if (hasPopup)
    {
        popup_ = MakeShared<Window>(context_);
        popup_->SetDefaultStyle(menu_->GetDefaultStyle());
        popup_->SetStyleAuto();
        popup_->SetLayout(LM_VERTICAL, 1, IntRect(2, 6, 2, 6));
        menu_->SetPopup(popup_);
        menu_->SetPopupOffset(0, menu_->GetHeight());
    }

    parent->AddChild(menu_);
}

AbstractMenu* UrhoMenu::AddMenu(const String& name)
{
    if (!popup_)
        return nullptr;
    children_.Push(MakeShared<UrhoMenu>(mainWindow_, popup_, name, "", true, false));
    return children_.Back();
}

AbstractMenu* UrhoMenu::AddAction(const String& name, const String& actionId)
{
    if (!popup_)
        return nullptr;
    children_.Push(MakeShared<UrhoMenu>(mainWindow_, popup_, name, actionId, false, false));
    return children_.Back();
}

void UrhoMenu::SetName(const String& name)
{
    text_->SetText(name);
}

void UrhoMenu::OnShowPopup()
{
    for (UrhoMenu* child : children_)
        if (child->onShown_)
            child->onShown_();
}

void UrhoMenu::HandleMenuSelected(StringHash eventType, VariantMap& eventData)
{
    if (menu_->GetPopup())
        return;

    mainWindow_->CollapseMenuPopups(menu_);

    if (actionCallback_)
        actionCallback_();
}

//////////////////////////////////////////////////////////////////////////
MenuWithPopupCallback::MenuWithPopupCallback(Context* context, UrhoMenu* abstractMenu)
    : Menu(context)
    , menu_(abstractMenu)
{

}

void MenuWithPopupCallback::OnShowPopup()
{
    menu_->OnShowPopup();
}

//////////////////////////////////////////////////////////////////////////
const Urho3D::StringHash UrhoMainWindow::VAR_DOCUMENT("Document");

UrhoMainWindow::UrhoMainWindow(Context* context)
    : AbstractMainWindow()
    , Object(context)
    , ui_(GetSubsystem<UI>())
    , uiRoot_(ui_->GetRoot())
    , mainElement_(new DockStation(context_))
    , input_(context_)
{
    Graphics* graphics = GetSubsystem<Graphics>();

    // Create cursor
    auto cursor = MakeShared<Cursor>(context_);
    cursor->SetStyleAuto();
    ui_->SetCursor(cursor);

    // Create dock station
    mainElement_->SetName("MainWindow");
    uiRoot_->AddChild(mainElement_);
    mainElement_->SetFixedSize(graphics->GetWidth(), graphics->GetHeight());

    SubscribeToEvent(E_SCREENMODE,
        [=](StringHash /*eventType*/, VariantMap& /*eventData*/)
    {
        mainElement_->SetFixedSize(graphics->GetWidth(), graphics->GetHeight());
    });

    // Create menu bar
    menuBar_ = new BorderImage(context_);
    menuBar_->SetName("MenuBar");
    mainElement_->AddDock(menuBar_, DockLocation::Top);
    menuBar_->SetLayout(LM_HORIZONTAL);
    menuBar_->SetStyle("EditorMenuBar");

    // Create document bar
    documentBar_ = new BorderImage(context_);
    documentBar_->SetName("DocumentBar");
    mainElement_->AddDock(documentBar_, DockLocation::Top);
    documentBar_->SetLayout(LM_HORIZONTAL);
    documentBar_->SetStyle("EditorMenuBar");

    documentList_ = documentBar_->CreateChild<DropDownList>("DocumentBar_List");
    documentList_->SetMaxWidth(200);
    documentList_->SetResizePopup(true);
    documentList_->SetStyleAuto();

    SubscribeToEvent(documentList_, E_ITEMSELECTED,
        [=](StringHash /*eventType*/, VariantMap& eventData)
    {
        if (UIElement* selectedElement = documentList_->GetSelectedItem())
        {
            if (Object* document = static_cast<Object*>(selectedElement->GetVar(VAR_DOCUMENT).GetPtr()))
            {
                if (onCurrentDocumentChanged_)
                    onCurrentDocumentChanged_(document);
            }
        }
    });

}

AbstractDock* UrhoMainWindow::AddDock(DockLocation hint, const IntVector2& sizeHint)
{
    UI* ui = GetSubsystem<UI>();
    UIElement* uiRoot = ui->GetRoot();

    auto dialog = MakeShared<UrhoDock>(this);
    dialog->SetSizeHint(sizeHint);
    mainElement_->AddDock(GetInternalElement(dialog), hint);
    dialog->SetParent(nullptr);
    dialogs_.Push(dialog);

    return dialog;
}

void UrhoMainWindow::AddAction(const AbstractAction& actionDesc)
{
    actions_[actionDesc.id_] = actionDesc;
}

AbstractMenu* UrhoMainWindow::AddMenu(const String& name)
{
    auto menu = MakeShared<UrhoMenu>(this, menuBar_, name, "", true, true);
    menus_.Push(menu);
    return menus_.Back();
}

void UrhoMainWindow::InsertDocument(Object* document, const String& title, unsigned index)
{
    documents_.Insert(SharedPtr<Object>(document));
    Text* documentTitle = documentList_->CreateChild<Text>();
    documentTitle->SetStyleAuto();
    documentTitle->SetText(title);
    documentTitle->SetVar(VAR_DOCUMENT, document);
    documentList_->AddItem(documentTitle);
    documentList_->SetMinHeight(documentTitle->GetMinHeight());

    // Notify if newly inserted item is selected
    if (documentList_->GetSelectedItem() == documentTitle)
    {
        if (onCurrentDocumentChanged_)
            onCurrentDocumentChanged_(document);
    }
}

void UrhoMainWindow::SelectDocument(Object* document)
{
    if (!documentList_)
        return;

    for (unsigned i = 0; i < documentList_->GetNumItems(); ++i)
    {
        UIElement* item = documentList_->GetItem(i);
        Object* itemDocument = static_cast<Object*>(item->GetVar(VAR_DOCUMENT).GetPtr());
        if (itemDocument == document)
        {
            documentList_->SetSelection(i);
            if (onCurrentDocumentChanged_)
                onCurrentDocumentChanged_(document);
        }
    }
}

PODVector<Object*> UrhoMainWindow::GetDocuments() const
{
    PODVector<Object*> result;
    const unsigned numDocuments = documentList_->GetNumItems();
    result.Resize(numDocuments);
    for (unsigned i = 0; i < numDocuments; ++i)
    {
        UIElement* item = documentList_->GetItem(i);
        result[i] = static_cast<Object*>(item->GetVar(VAR_DOCUMENT).GetPtr());
    }
    return result;
}

AbstractAction* UrhoMainWindow::FindAction(const String& actionId) const
{
    return actions_[actionId];
}

void UrhoMainWindow::CollapseMenuPopups(Menu* menu) const
{
    // Go to topmost menu
    while (UIElement* parent = menu->GetParent())
    {
        Menu* parentMenu = dynamic_cast<Menu*>(parent->GetVar("Origin").GetPtr());
        if (parentMenu)
            menu = parentMenu;
        else
            break;
    }

    if (menu->GetParent() == menuBar_)
        menu->ShowPopup(false);
}

}
