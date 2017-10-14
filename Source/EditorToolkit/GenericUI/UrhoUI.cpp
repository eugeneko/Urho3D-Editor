#include "UrhoUI.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Menu.h>
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
    window_->SetMinSize(200, 200);
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

void UrhoDialog::SetName(const String& name)
{
    windowTitle_->SetText(name);
}

bool UrhoDialog::SetContent(GenericWidget* content)
{
    // Test element
    auto urhoWidget = dynamic_cast<UrhoWidget*>(content);
    if (!urhoWidget)
        return false;

    bodyElement_->RemoveAllChildren();
    urhoWidget->CreateElement(bodyElement_);
    return true;
}

//////////////////////////////////////////////////////////////////////////
UrhoScrollArea::UrhoScrollArea(AbstractMainWindow& mainWindow, GenericWidget* parent)
    : AbstractScrollArea(mainWindow, parent)
{

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

bool UrhoScrollArea::SetContent(GenericWidget* content)
{
    // Test element
    auto urhoWidget = dynamic_cast<UrhoWidget*>(content);
    if (!urhoWidget)
        return false;

    // Set content
    scrollView_->SetContentElement(urhoWidget->CreateElement(scrollPanel_));
    UpdateContentSize();
    return true;
}

UIElement* UrhoScrollArea::CreateElement(UIElement* parent)
{
    scrollView_ = parent->CreateChild<ScrollView>("ASR_ScrollView");
    scrollView_->SetStyleAuto();

    scrollPanel_ = scrollView_->GetScrollPanel();
    SubscribeToEvent(scrollPanel_, E_LAYOUTUPDATED, URHO3D_HANDLER(UrhoScrollArea, HandleResized));
    return scrollView_;
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
        body->SetFixedWidth(scrollPanel_->GetWidth() - clipBorder.left_ - clipBorder.right_);
    }
    --layoutNestingLevel_;
}

//////////////////////////////////////////////////////////////////////////
UrhoLayout::UrhoLayout(AbstractMainWindow& mainWindow, GenericWidget* parent)
    : AbstractLayout(mainWindow, parent)
{
}

void UrhoLayout::UpdateLayout()
{
    // Disable layout update
    body_->DisableLayoutUpdate();

    Vector<int> minRowHeight;
    Vector<int> minColumnWidth;
    Vector<int> maxColumnWidth;

    // Compute sizes
    const unsigned numRows = elements_.Size();
    minRowHeight.Resize(numRows, 0);
    for (unsigned row = 0; row < numRows; ++row)
    {
        const RowType rowType = elements_[row].second_;
        const Vector<UIElement*>& rowElements = elements_[row].first_;
        const unsigned numColumns = rowElements.Size();
        if (minColumnWidth.Size() <= numColumns)
            minColumnWidth.Resize(numColumns, 0);
        if (maxColumnWidth.Size() <= numColumns)
            maxColumnWidth.Resize(numColumns, 0);
        for (unsigned column = 0; column < numColumns; ++column)
        {
            UIElement* cellElement = rowElements[column];
            if (!cellElement /*|| cellElement->GetNumChildren() == 0*/)
                continue;
            const IntVector2 cellSize = cellElement->GetEffectiveMinSize();
            const IntVector2 maxCellSize = cellElement->GetMaxSize();

            if (rowType != RowType::SingleColumn)
            {
                minColumnWidth[column] = Max(minColumnWidth[column], cellSize.x_);
                maxColumnWidth[column] = Max(maxColumnWidth[column], maxCellSize.x_);
            }
            minRowHeight[row] = Max(minRowHeight[row], cellSize.y_);
        }
    }

    // Compute width stretch
    const unsigned numColumns = minColumnWidth.Size();
    const int minBodyWidth = std::accumulate(&*minColumnWidth.Begin(), &*minColumnWidth.End(), 0);

    Vector<int> maxColumnStretch;
    maxColumnStretch.Resize(numColumns, 0);
    for (unsigned i = 0; i < numColumns; ++i)
        maxColumnStretch[i] = Max(0, maxColumnWidth[i] - minColumnWidth[i]);

    Vector<int> columnWidth = minColumnWidth;
    int remainingStretch = Max(0, body_->GetWidth() - minBodyWidth);
    unsigned remainingColumns = numColumns;
    while (remainingStretch > 0 && remainingColumns > 0)
    {
        // Compute number of stretchable columns
        remainingColumns = 0;
        for (unsigned i = 0; i < numColumns; ++i)
            if (columnWidth[i] < maxColumnWidth[i])
                ++remainingColumns;
        if (remainingColumns == 0)
            break;

        // Stretch columns
        const int columnDelta = (remainingStretch + remainingColumns - 1) / remainingColumns;
        for (unsigned i = 0; i < numColumns; ++i)
        {
            if (columnWidth[i] < maxColumnWidth[i])
            {
                const int oldWidth = columnWidth[i];
                columnWidth[i] = Min(maxColumnWidth[i], columnWidth[i] + Min(columnDelta, remainingStretch));
                remainingStretch -= columnWidth[i] - oldWidth;
            }
        }
    }

    // Update body height
    int bodyHeight = 0;
    for (unsigned row = 0; row < numRows; ++row)
        bodyHeight += minRowHeight[row];
    body_->SetMinHeight(bodyHeight);
    body_->SetHeight(bodyHeight);

    // Apply sizes
    IntVector2 position;
    for (unsigned row = 0; row < numRows; ++row)
    {
        const RowType rowType = elements_[row].second_;
        const Vector<UIElement*>& rowElements = elements_[row].first_;
        const int rowHeight = minRowHeight[row];
        position.x_ = 0;
        if (rowType == RowType::SingleColumn)
        {
            // Create single column
            UIElement* cellElement = rowElements[0];
            if (!cellElement)
                continue;
            cellElement->SetPosition(position);
            cellElement->SetWidth(body_->GetWidth());
            position.x_ += cellElement->GetWidth();
        }
        else
        {
            // Iterate over columns
            for (unsigned column = 0; column < rowElements.Size(); ++column)
            {
                UIElement* cellElement = rowElements[column];
                if (!cellElement)
                    continue;

                const int width = columnWidth[column];
                cellElement->SetPosition(position);
                cellElement->SetSize(width, rowHeight);
                position.x_ += width;
            }
        }
        position.y_ += rowHeight;
    }

    // Update layout
    body_->EnableLayoutUpdate();
    body_->UpdateLayout();
}

UIElement* UrhoLayout::CreateElement(UIElement* parent)
{
    body_ = parent->CreateChild<UIElement>("AL_Content");
    SubscribeToEvent(body_, E_LAYOUTUPDATED, URHO3D_HANDLER(UrhoLayout, HandleLayoutChanged));
    //SubscribeToEvent(body_, E_VISIBLECHANGED, URHO3D_HANDLER(UrhoLayout, HandleLayoutChanged));
    return body_;
}

bool UrhoLayout::SetCellWidget(unsigned row, unsigned column, GenericWidget* child)
{
    // Test element
    auto urhoWidget = dynamic_cast<UrhoWidget*>(child);
    if (!urhoWidget)
    {
        URHO3D_LOGERRORF("Cannot add unknown widget into row %u column %u", row, column);
        return false;
    }

    // Populate row
    if (elements_.Size() <= row)
        elements_.Resize(row + 1);

    elements_[row].second_ = RowType::MultipleColumns;
    Vector<UIElement*>& rowElements = elements_[row].first_;
    if (rowElements.Size() <= column)
        rowElements.Resize(column + 1);

    // Add widget
    if (rowElements[column])
        body_->RemoveChild(rowElements[column]);
    rowElements[column] = urhoWidget->CreateElement(body_);

    // Update layout
    UpdateLayout();
    return true;
}

bool UrhoLayout::SetRowWidget(unsigned row, GenericWidget* child)
{
    // Test element
    auto urhoWidget = dynamic_cast<UrhoWidget*>(child);
    if (!urhoWidget)
    {
        URHO3D_LOGERRORF("Cannot add unknown widget into row %u", row);
        return false;
    }

    // Populate row
    if (elements_.Size() <= row)
        elements_.Resize(row + 1);

    elements_[row].second_ = RowType::SingleColumn;
    Vector<UIElement*>& rowElements = elements_[row].first_;
    for (UIElement* cellElement : rowElements)
        body_->RemoveChild(cellElement);
    rowElements.Resize(1);

    // Add widget
    rowElements[0] = urhoWidget->CreateElement(body_);

    // Update layout
    UpdateLayout();
    return true;
}

void UrhoLayout::HandleLayoutChanged(StringHash /*eventType*/, VariantMap& /*eventData*/)
{
    UpdateLayout();
}

//////////////////////////////////////////////////////////////////////////
void UrhoCollapsiblePanel::SetHeaderText(const String& text)
{
    headerText_->SetText(text);
}

void UrhoCollapsiblePanel::SetExpanded(bool expanded)
{
    toggleButton_->SetChecked(expanded);
    if (body_)
        body_->SetVisible(expanded);
    panel_->SetHeight(panel_->GetEffectiveMinSize().y_);
    //panel_->GetParent()->UpdateLayout();
}

bool UrhoCollapsiblePanel::SetHeaderPrefix(GenericWidget* header)
{
    // Test element
    auto headerImpl = dynamic_cast<UrhoWidget*>(header);
    if (!headerImpl)
        return false;

    headerPrefix_->RemoveAllChildren();
    headerImpl->CreateElement(headerPrefix_);

    return true;
}

bool UrhoCollapsiblePanel::SetHeaderSuffix(GenericWidget* header)
{
    // Test element
    auto headerImpl = dynamic_cast<UrhoWidget*>(header);
    if (!headerImpl)
        return false;

    headerSuffix_->RemoveAllChildren();
    headerImpl->CreateElement(headerSuffix_);

    return true;
}

bool UrhoCollapsiblePanel::SetBody(GenericWidget* body)
{
    // Test element
    auto bodyImpl = dynamic_cast<UrhoWidget*>(body);
    if (!bodyImpl)
        return false;

    if (body_)
        panel_->RemoveChild(body_);
    body_ = bodyImpl->CreateElement(panel_);

    return true;
}

UIElement* UrhoCollapsiblePanel::CreateElement(UIElement* parent)
{
    panel_ = parent->CreateChild<BorderImage>("CP_Panel");
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
    return panel_;
}

//////////////////////////////////////////////////////////////////////////
AbstractButton& UrhoButton::SetText(const String& text)
{
    text_->SetText(text);
    UpdateContainerSize();
    return *this;
}

UIElement* UrhoButton::CreateElement(UIElement* parent)
{
    button_ = parent->CreateChild<Button>();
    button_->SetStyleAuto();
    button_->SetClipChildren(true);
    text_ = button_->CreateChild<Text>();
    text_->SetStyleAuto();
    text_->SetAlignment(HA_CENTER, VA_CENTER);
    text_->SetMinHeight(static_cast<int>(text_->GetRowHeight()));
    UpdateContainerSize();
    return button_;
}

void UrhoButton::UpdateContainerSize()
{
    button_->SetMinSize(text_->GetMinHeight(), text_->GetMinHeight());
    button_->SetMaxWidth(Max(text_->GetMinWidth(), text_->GetMinHeight()) + 4);
    button_->SetMaxHeight(text_->GetMinHeight() + 4);
}

//////////////////////////////////////////////////////////////////////////
AbstractText& UrhoText::SetText(const String& text)
{
    text_->SetText(text);
    container_->SetFixedSize(text_->GetMinSize());
    return *this;
}

UIElement* UrhoText::CreateElement(UIElement* parent)
{
    container_ = parent->CreateChild<UIElement>();
    container_->SetClipChildren(true);
    text_ = container_->CreateChild<Text>();
    text_->SetStyleAuto();
    return container_;
}

//////////////////////////////////////////////////////////////////////////
AbstractLineEdit& UrhoLineEdit::SetText(const String& text)
{
    lineEdit_->SetText(text);
    return *this;
}

UIElement* UrhoLineEdit::CreateElement(UIElement* parent)
{
    lineEdit_ = parent->CreateChild<LineEdit>();
    lineEdit_->SetStyleAuto();
    const int defaultHeight = static_cast<int>(lineEdit_->GetTextElement()->GetRowHeight());
    lineEdit_->SetMinSize(defaultHeight * 2, defaultHeight);
    return lineEdit_;
}

//////////////////////////////////////////////////////////////////////////
AbstractCheckBox& UrhoCheckBox::SetChecked(bool checked)
{
    checkBox_->SetChecked(checked);
    return *this;
}

UIElement* UrhoCheckBox::CreateElement(UIElement* parent)
{
    panel_ = parent->CreateChild<UIElement>("ACB_Panel");
    panel_->SetLayout(LM_HORIZONTAL);
    checkBox_ = panel_->CreateChild<CheckBox>("ACB_CheckBox");
    checkBox_->SetStyleAuto();
    text_ = panel_->CreateChild<Text>("ACB_Text");
    text_->SetStyleAuto();
    return panel_;
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

UIElement* UrhoHierarchyList::CreateElement(UIElement* parent)
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
    return hierarchyList_;
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
