#include "UrhoUI.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Menu.h>
#include <Urho3D/UI/LineEdit.h>

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

}

//////////////////////////////////////////////////////////////////////////
UrhoDialog::UrhoDialog(AbstractMainWindow& mainWindow, GenericWidget* parent)
    : GenericDialog(mainWindow, parent)
{
    UI* ui = GetSubsystem<UI>();
    UIElement* uiRoot = ui->GetRoot();

    // Create window
    window_ = uiRoot->CreateChild<Window>();
    window_->SetStyleAuto();
    window_->SetPosition(200, 200);
    window_->SetSize(500, 200);
    window_->SetResizeBorder(IntRect(6, 6, 6, 6));
    window_->SetResizable(true);
    window_->SetMovable(true);
    window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    window_->SetName("Window");

    // Create title
    UIElement* titleBar = window_->CreateChild<UIElement>();
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    windowTitle_ = titleBar->CreateChild<Text>();
    windowTitle_->SetStyleAuto();
    windowTitle_->SetName("WindowTitle");

    Button* buttonClose = titleBar->CreateChild<Button>();
    buttonClose->SetStyle("CloseButton");
    buttonClose->SetName("CloseButton");

    titleBar->SetFixedHeight(titleBar->GetMinHeight());

    bodyElement_ = window_->CreateChild<UIElement>();
    bodyElement_->SetLayoutMode(LM_VERTICAL);
}

void UrhoDialog::SetBodyWidget(GenericWidget* widget)
{
    bodyElement_->RemoveAllChildren();
    if (auto urhoWidget = dynamic_cast<UrhoWidget*>(widget))
    {
        body_ = widget;
        urhoWidget->CreateElements(bodyElement_);
    }
}

void UrhoDialog::SetName(const String& name)
{
    windowTitle_->SetText(name);
}

//////////////////////////////////////////////////////////////////////////
UrhoLayout::UrhoLayout(AbstractMainWindow& mainWindow, GenericWidget* parent)
    : AbstractLayout(mainWindow, parent)
{
}

void UrhoLayout::ResetRows()
{
    children_.Clear();
    rowElements_.Clear();
}

void UrhoLayout::AddFixedColumn(float width, int minWidth)
{
    ResetRows();
    ColumnDesc desc;
    desc.fixed_ = true;
    desc.width_ = width;
    desc.minWidth_ = minWidth;
    columns_.Push(desc);
}

void UrhoLayout::AddColumn()
{
    ResetRows();
    ColumnDesc desc;
    desc.fixed_ = false;
    columns_.Push(desc);
}

GenericWidget& UrhoLayout::CreateCellWidget(StringHash type, unsigned row, unsigned column)
{
    SharedPtr<GenericWidget> child = mainWindow_.CreateWidget(type, this);
    const bool success = AddCellWidget(row, column, child);
    assert(success);
    return *child;
}

GenericWidget& UrhoLayout::CreateRowWidget(StringHash type, unsigned row)
{
    SharedPtr<GenericWidget> child = mainWindow_.CreateWidget(type, this);
    const bool success = AddRowWidget(row, child);
    assert(success);
    return *child;
}

void UrhoLayout::UpdateLayout()
{
    for (unsigned column = 0; column < columns_.Size(); ++column)
    {
        const ColumnDesc& desc = columns_[column];
        if (!desc.fixed_)
            continue;

        const int columnWidth = Max(static_cast<int>(body_->GetWidth() * desc.width_), desc.minWidth_);
        for (unsigned row = 0; row < rowElements_.Size(); ++row)
        {
            const RowType rowType = rowElements_[row].second_;
            if (rowElements_[row].second_ != RowType::MultipleColumns)
                continue;
            UIElement* rowElement = rowElements_[row].first_;
            if (column < rowElement->GetNumChildren())
            {
                UIElement* cellElement = rowElement->GetChild(column);
                cellElement->SetFixedWidth(columnWidth);
            }
        }
    }
    body_->UpdateLayout();
}

void UrhoLayout::CreateElements(UIElement* parent)
{
    body_ = parent->CreateChild<UIElement>("LayoutBody");
    SubscribeToEvent(body_, E_LAYOUTUPDATED, URHO3D_HANDLER(UrhoLayout, HandleLayoutChanged));
    body_->SetLayout(LM_VERTICAL);

    AddFixedColumn(0.35f, 100);
    AddColumn();
    CreateCellWidget<AbstractText>(0, 0).SetText("Position");
    CreateCellWidget<AbstractText>(0, 1).SetText("X");
    CreateCellWidget<AbstractLineEdit>(0, 1).SetText("1");
    CreateCellWidget<AbstractText>(0, 1).SetText("Y");
    CreateCellWidget<AbstractLineEdit>(0, 1).SetText("2");
    CreateCellWidget<AbstractText>(0, 1).SetText("Z");
    CreateCellWidget<AbstractLineEdit>(0, 1).SetText("3");
    CreateCellWidget<AbstractText>(1, 0).SetText("Some long long long name");
    CreateCellWidget<AbstractLineEdit>(1, 1).SetText("Some long long long edit");

    CreateRowWidget<AbstractButton>(2).SetText("Build");
}

bool UrhoLayout::AddCellWidget(unsigned row, unsigned column, GenericWidget* childWidget)
{
    // Save child
    children_.Push(SharedPtr<GenericWidget>(childWidget));

    // Populate row
    for (unsigned rowIdx = rowElements_.Size(); rowIdx <= row; ++rowIdx)
    {
        UIElement* rowElement = body_->CreateChild<UIElement>("LayoutRow");
        rowElement->SetLayout(LM_HORIZONTAL);
        rowElements_.Push(MakePair(rowElement, RowType::MultipleColumns));

        for (unsigned columnIdx = 0; columnIdx < columns_.Size(); ++columnIdx)
        {
            UIElement* cellElement = rowElement->CreateChild<UIElement>("LayoutCell");
            cellElement->SetLayout(LM_HORIZONTAL);
        }
    }

    // Test element
    auto urhoWidget = dynamic_cast<UrhoWidget*>(childWidget);
    if (!urhoWidget)
    {
        URHO3D_LOGERRORF("Cannot add unknown widget into row %u column %u", row, column);
        return false;
    }

    // Test row
    const RowType rowType = rowElements_[row].second_;
    UIElement* rowElement = rowElements_[row].first_;
    if (rowType != RowType::MultipleColumns || column >= rowElement->GetNumChildren())
    {
        URHO3D_LOGERRORF("Cannot add cell widget into row %u column %u", row, column);
        return false;
    }

    // Add widget
    urhoWidget->CreateElements(rowElement->GetChild(column));
    rowElement->UpdateLayout();

    // Update layout
    UpdateLayout();
    return true;
}

bool UrhoLayout::AddRowWidget(unsigned row, GenericWidget* childWidget)
{
    for (unsigned rowIdx = rowElements_.Size(); rowIdx <= row; ++rowIdx)
    {
        UIElement* rowElement = body_->CreateChild<UIElement>("LayoutRow");
        rowElement->SetLayout(LM_HORIZONTAL);
        rowElements_.Push(MakePair(rowElement, RowType::SingleColumn));
    }

    // Test element
    auto urhoWidget = dynamic_cast<UrhoWidget*>(childWidget);
    if (!urhoWidget)
    {
        URHO3D_LOGERRORF("Cannot add unknown widget into row %u", row);
        return false;
    }

    // Test row
    const RowType rowType = rowElements_[row].second_;
    if (rowType != RowType::SingleColumn)
    {
        URHO3D_LOGERRORF("Cannot add row widget into row %u", row);
        return false;
    }

    // Reset row content
    UIElement* rowElement = rowElements_[row].first_;
    rowElement->RemoveAllChildren();
    urhoWidget->CreateElements(rowElement);
    rowElement->UpdateLayout();
    children_.Push(SharedPtr<GenericWidget>(childWidget));
    return true;
}

void UrhoLayout::HandleLayoutChanged(StringHash /*eventType*/, VariantMap& /*eventData*/)
{
    UpdateLayout();
}

//////////////////////////////////////////////////////////////////////////
AbstractButton& UrhoButton::SetText(const String& text)
{
    text_->SetText(text);
    return *this;
}

void UrhoButton::CreateElements(UIElement* parent)
{
    button_ = parent->CreateChild<Button>();
    button_->SetStyleAuto();
    button_->SetClipChildren(true);
    text_ = button_->CreateChild<Text>();
    text_->SetStyleAuto();
    text_->SetAlignment(HA_CENTER, VA_CENTER);
}

//////////////////////////////////////////////////////////////////////////
AbstractText& UrhoText::SetText(const String& text)
{
    text_->SetText(text);
    return *this;
}

void UrhoText::CreateElements(UIElement* parent)
{
    text_ = parent->CreateChild<Text>();
    text_->SetStyleAuto();
}

//////////////////////////////////////////////////////////////////////////
AbstractLineEdit& UrhoLineEdit::SetText(const String& text)
{
    lineEdit_->SetText(text);
    return *this;
}

void UrhoLineEdit::CreateElements(UIElement* parent)
{
    lineEdit_ = parent->CreateChild<LineEdit>();
    lineEdit_->SetStyleAuto();
}

//////////////////////////////////////////////////////////////////////////
UrhoHierarchyList::UrhoHierarchyList(AbstractMainWindow& mainWindow, GenericWidget* parent)
    : GenericHierarchyList(mainWindow, parent)
    , rootItem_(context_)
{
}

void UrhoHierarchyList::AddItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent)
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

void UrhoHierarchyList::SelectItem(GenericHierarchyListItem* item)
{
    if (auto itemWidget = dynamic_cast<UIElement*>(item->GetInternalPointer()))
    {
        const unsigned index = hierarchyList_->FindItem(itemWidget);
        if (!hierarchyList_->IsSelected(index))
            hierarchyList_->ToggleSelection(index);
    }
}

void UrhoHierarchyList::DeselectItem(GenericHierarchyListItem* item)
{
    if (auto itemWidget = dynamic_cast<UIElement*>(item->GetInternalPointer()))
    {
        const unsigned index = hierarchyList_->FindItem(itemWidget);
        if (hierarchyList_->IsSelected(index))
            hierarchyList_->ToggleSelection(index);
    }
}

void UrhoHierarchyList::GetSelection(ItemVector& result)
{
    for (unsigned index : hierarchyList_->GetSelections())
    {
        UIElement* element = hierarchyList_->GetItem(index);
        if (auto item = dynamic_cast<UrhoHierarchyListItemWidget*>(element))
            result.Push(item->GetItem());
    }
}

void UrhoHierarchyList::CreateElements(UIElement* parent)
{
    hierarchyList_ = parent->CreateChild<ListView>();
    hierarchyList_->SetInternal(true);
    hierarchyList_->SetName("HierarchyList");
    hierarchyList_->SetHighlightMode(HM_ALWAYS);
    hierarchyList_->SetMultiselect(true);
    hierarchyList_->SetSelectOnClickEnd(true);
    hierarchyList_->SetHierarchyMode(true);
    hierarchyList_->SetStyle("HierarchyListView");
    SubscribeToEvent(hierarchyList_, E_ITEMCLICKED, URHO3D_HANDLER(UrhoHierarchyList, HandleItemClicked));
}

void UrhoHierarchyList::InsertItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent)
{
    auto itemWidget = MakeShared<UrhoHierarchyListItemWidget>(context_, item);
    itemWidget->SetText(item->GetText());
    item->SetInternalPointer(itemWidget);

    UIElement* parentWidget = parent ? dynamic_cast<UIElement*>(parent->GetInternalPointer()) : nullptr;
    if (itemWidget)
    {
        hierarchyList_->InsertItem(M_MAX_UNSIGNED, itemWidget, parentWidget);
        for (unsigned i = 0; i < item->GetNumChildren(); ++i)
            InsertItem(item->GetChild(i), M_MAX_UNSIGNED, item);
    }
}

void UrhoHierarchyList::HandleItemClicked(StringHash /*eventType*/, VariantMap& eventData)
{
    RefCounted* element = eventData[ItemClicked::P_ITEM].GetPtr();
    if (auto item = dynamic_cast<UrhoHierarchyListItemWidget*>(element))
    {
        SendEvent(E_GENERICWIDGETCLICKED,
            GenericWidgetClicked::P_ELEMENT, this,
            GenericWidgetClicked::P_ITEM, item->GetItem());
    }
}

//////////////////////////////////////////////////////////////////////////
UrhoHierarchyListItemWidget::UrhoHierarchyListItemWidget(Context* context, GenericHierarchyListItem* item)
    : Text(context)
    , item_(item)
{
    SetStyle("FileSelectorListText");
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
UrhoMenu::UrhoMenu(UrhoMainWindow& mainWindow, UIElement* parent, const String& text, const String& actionId,
    bool hasPopup, bool topLevel)
    : Object(mainWindow.GetContext())
    , mainWindow_(mainWindow)
{
    AbstractAction* action = nullptr;
    if (!actionId.Empty())
    {
        action = mainWindow_.FindAction(actionId);
    }

    menu_ = new Menu(context_);
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

GenericMenu* UrhoMenu::AddMenu(const String& name)
{
    if (!popup_)
        return nullptr;
    children_.Push(MakeShared<UrhoMenu>(mainWindow_, popup_, name, "", true, false));
    return children_.Back();
}

GenericMenu* UrhoMenu::AddAction(const String& name, const String& actionId)
{
    if (!popup_)
        return nullptr;
    children_.Push(MakeShared<UrhoMenu>(mainWindow_, popup_, name, actionId, false, false));
    return children_.Back();
}

void UrhoMenu::HandleMenuSelected(StringHash eventType, VariantMap& eventData)
{
    if (menu_->GetPopup())
        return;

    mainWindow_.CollapseMenuPopups(menu_);

    if (actionCallback_)
        actionCallback_();
}

//////////////////////////////////////////////////////////////////////////
UrhoMainWindow::UrhoMainWindow(Context* context)
    : AbstractMainWindow()
    , Object(context)
    , input_(context)
{
    SubscribeToEvent(E_SCREENMODE, URHO3D_HANDLER(UrhoMainWindow, HandleResized));
}

GenericDialog* UrhoMainWindow::AddDialog(DialogLocationHint hint)
{
    dialogs_.Push(MakeShared<UrhoDialog>(*this, nullptr));
    return dialogs_.Back();
}

void UrhoMainWindow::AddAction(const AbstractAction& actionDesc)
{
    actions_[actionDesc.id_] = actionDesc;
}

GenericMenu* UrhoMainWindow::AddMenu(const String& name)
{
    if (!menuBar_)
    {
        UI* ui = GetSubsystem<UI>();
        Graphics* graphics = GetSubsystem<Graphics>();
        menuBar_ = ui->GetRoot()->CreateChild<BorderImage>("MenuBar");
        menuBar_->SetLayout(LM_HORIZONTAL);
        menuBar_->SetFixedWidth(graphics->GetWidth());
        menuBar_->SetStyle("EditorMenuBar");
    }

    menus_.Push(MakeShared<UrhoMenu>(*this, menuBar_, name, "", true, true));
    return menus_.Back();
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

void UrhoMainWindow::HandleResized(StringHash eventType, VariantMap& eventData)
{
    if (menuBar_)
    {
        Graphics* graphics = GetSubsystem<Graphics>();
        menuBar_->SetFixedWidth(graphics->GetWidth());
    }
}

}
