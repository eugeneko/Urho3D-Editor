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

UIElement* GetParentElement(AbstractWidget* widget)
{
    return GetInternalElement(widget->GetParent());
}

}

//////////////////////////////////////////////////////////////////////////
UrhoDock::UrhoDock(AbstractMainWindow& mainWindow)
    : AbstractDock(mainWindow)
{
}

void UrhoDock::SetName(const String& name)
{
    windowTitle_->SetText(name);
}

void UrhoDock::OnParentSet()
{
    // #TODO Make main window root widget
    UI* ui = GetSubsystem<UI>();
    UIElement* uiRoot = ui->GetRoot();

    // Create window
    window_ = uiRoot->CreateChild<Window>("AD_Window");
    window_->SetStyleAuto();
    window_->SetPosition(200, 200);
    window_->SetMinSize(200, 200);
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

    SetInternalElement(this, window_);
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
UrhoScrollArea::UrhoScrollArea(AbstractMainWindow& mainWindow)
    : AbstractScrollArea(mainWindow)
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

bool UrhoScrollArea::DoSetContent(AbstractWidget* content)
{
    if (!GetInternalElement(content))
        return false;

    scrollView_->SetContentElement(GetInternalElement(content));
    return true;
}

void UrhoScrollArea::OnParentSet()
{
    UIElement* parent = GetParentElement(this);

    scrollView_ = parent->CreateChild<ScrollView>("ASR_ScrollView");
    scrollView_->SetStyleAuto();

    scrollPanel_ = scrollView_->GetScrollPanel();
    SubscribeToEvent(scrollPanel_, E_LAYOUTUPDATED, URHO3D_HANDLER(UrhoScrollArea, HandleResized));

    SetInternalElement(this, scrollView_);
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
UrhoLayout::UrhoLayout(AbstractMainWindow& mainWindow)
    : AbstractLayout(mainWindow)
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
    const unsigned numRows = rows_.Size();
    minRowHeight.Resize(numRows, 0);
    for (unsigned row = 0; row < numRows; ++row)
    {
        const RowType rowType = rows_[row].type_;
        const unsigned numColumns = rows_[row].columns_.Size();
        if (minColumnWidth.Size() <= numColumns)
            minColumnWidth.Resize(numColumns, 0);
        if (maxColumnWidth.Size() <= numColumns)
            maxColumnWidth.Resize(numColumns, 0);
        for (unsigned column = 0; column < numColumns; ++column)
        {
            UIElement* cellElement = GetInternalElement(rows_[row].columns_[column]);
            if (!cellElement /*|| cellElement->GetNumChildren() == 0*/)
                continue;
            const IntVector2 cellSize = cellElement->GetEffectiveMinSize();
            const IntVector2 maxCellSize = cellElement->GetMaxSize();

            if (rowType != RowType::SimpleRow)
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
        const RowType rowType = rows_[row].type_;
        const int rowHeight = minRowHeight[row];
        position.x_ = 0;
        if (rowType == RowType::SimpleRow)
        {
            // Create single column
            UIElement* cellElement = GetInternalElement(rows_[row].columns_[0]);
            if (!cellElement)
                continue;
            cellElement->SetPosition(position);
            cellElement->SetWidth(body_->GetWidth());
            position.x_ += cellElement->GetWidth();
        }
        else if (rowType == AbstractLayout::RowType::MultiCellRow)
        {
            // Iterate over columns
            for (unsigned column = 0; column < rows_[row].columns_.Size(); ++column)
            {
                UIElement* cellElement = GetInternalElement(rows_[row].columns_[column]);
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

void UrhoLayout::OnParentSet()
{
    UIElement* parent = GetParentElement(this);

    body_ = parent->CreateChild<UIElement>("AL_Content");
    SubscribeToEvent(body_, E_LAYOUTUPDATED, URHO3D_HANDLER(UrhoLayout, HandleLayoutChanged));

    SetInternalElement(this, body_);
}

bool UrhoLayout::DoSetCell(unsigned row, unsigned column, AbstractWidget* child)
{
    if (!GetInternalElement(child))
        return false;

    body_->AddChild(GetInternalElement(child));
    UpdateLayout();
    return true;
}

bool UrhoLayout::DoSetRow(unsigned row, AbstractWidget* child)
{
    if (!GetInternalElement(child))
        return false;

    body_->AddChild(GetInternalElement(child));
    UpdateLayout();
    return true;
}

void UrhoLayout::DoRemoveChild(AbstractWidget* child)
{
    if (UIElement* childElement = GetInternalElement(child))
        body_->RemoveChild(childElement);
    SetInternalElement(child, nullptr);
}

void UrhoLayout::HandleLayoutChanged(StringHash /*eventType*/, VariantMap& /*eventData*/)
{
    UpdateLayout();
}

//////////////////////////////////////////////////////////////////////////
UrhoCollapsiblePanel::UrhoCollapsiblePanel(AbstractMainWindow& mainWindow)
    : AbstractCollapsiblePanel(mainWindow)
{
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

    UpdateContentSize();

    return true;
}

void UrhoCollapsiblePanel::UpdateContentSize()
{
    if (body_)
        body_->SetVisible(toggleButton_->IsChecked());
    panel_->SetHeight(panel_->GetEffectiveMinSize().y_);
}

void UrhoCollapsiblePanel::OnParentSet()
{
    UIElement* parent = GetParentElement(this);

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

    SetInternalElement(this, panel_);
}

//////////////////////////////////////////////////////////////////////////
UrhoButton::UrhoButton(AbstractMainWindow& mainWindow)
    : AbstractButton(mainWindow)
{
}

void UrhoButton::SetText(const String& text)
{
    text_->SetText(text);
    UpdateButtonSize();
}

void UrhoButton::OnParentSet()
{
    UIElement* parent = GetParentElement(this);

    button_ = parent->CreateChild<Button>();
    button_->SetStyleAuto();
    button_->SetClipChildren(true);

    text_ = button_->CreateChild<Text>();
    text_->SetStyleAuto();
    text_->SetAlignment(HA_CENTER, VA_CENTER);
    text_->SetMinHeight(static_cast<int>(text_->GetRowHeight()));

    UpdateButtonSize();

    SetInternalElement(this, button_);
}

void UrhoButton::UpdateButtonSize()
{
    IntVector2 size = text_->GetMinSize() + IntVector2(4, 4);
    size.x_ = Max(size.x_, size.y_);
    button_->SetFixedSize(size);
}

//////////////////////////////////////////////////////////////////////////
UrhoText::UrhoText(AbstractMainWindow& mainWindow)
    : AbstractText(mainWindow)
{
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
    UIElement* parent = GetParentElement(this);

    text_ = parent->CreateChild<Text>();
    text_->SetStyleAuto();

    SetInternalElement(this, text_);
}

//////////////////////////////////////////////////////////////////////////
UrhoLineEdit::UrhoLineEdit(AbstractMainWindow& mainWindow)
    : AbstractLineEdit(mainWindow)
{
}

void UrhoLineEdit::SetText(const String& text)
{
    suppressTextChange_ = true;
    lineEdit_->SetText(text);
    suppressTextChange_ = false;
}

const String& UrhoLineEdit::GetText() const
{
    return lineEdit_->GetText();
}

void UrhoLineEdit::OnParentSet()
{
    UIElement* parent = GetParentElement(this);

    lineEdit_ = parent->CreateChild<LineEdit>();
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

    SetInternalElement(this, lineEdit_);
}

//////////////////////////////////////////////////////////////////////////
UrhoCheckBox::UrhoCheckBox(AbstractMainWindow& mainWindow)
    : AbstractCheckBox(mainWindow)
{
}

void UrhoCheckBox::SetChecked(bool checked)
{
    checkBox_->SetChecked(checked);
}

void UrhoCheckBox::OnParentSet()
{
    UIElement* parent = GetParentElement(this);

    panel_ = parent->CreateChild<UIElement>("ACB_Panel");
    panel_->SetLayout(LM_HORIZONTAL);
    checkBox_ = panel_->CreateChild<CheckBox>("ACB_CheckBox");
    text_ = panel_->CreateChild<Text>("ACB_Text");

    SetInternalElement(this, panel_);

    checkBox_->SetStyleAuto();
    text_->SetStyleAuto();
}

//////////////////////////////////////////////////////////////////////////
UrhoHierarchyList::UrhoHierarchyList(AbstractMainWindow& mainWindow)
    : AbstractHierarchyList(mainWindow)
    , rootItem_(context_)
{
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

void UrhoHierarchyList::SelectItem(AbstractHierarchyListItem* item)
{
    if (auto itemWidget = dynamic_cast<UIElement*>(item->GetInternalPointer()))
    {
        const unsigned index = hierarchyList_->FindItem(itemWidget);
        if (!hierarchyList_->IsSelected(index))
            hierarchyList_->ToggleSelection(index);
    }
}

void UrhoHierarchyList::DeselectItem(AbstractHierarchyListItem* item)
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

void UrhoHierarchyList::OnParentSet()
{
    UIElement* parent = GetParentElement(this);

    hierarchyList_ = parent->CreateChild<ListView>("AHL_HierarchyList");
    hierarchyList_->SetStyle("HierarchyListView");
    hierarchyList_->SetInternal(true);
    hierarchyList_->SetHighlightMode(HM_ALWAYS);
    hierarchyList_->SetMultiselect(true);
    hierarchyList_->SetSelectOnClickEnd(true);
    hierarchyList_->SetHierarchyMode(true);
    SubscribeToEvent(hierarchyList_, E_ITEMCLICKED, URHO3D_HANDLER(UrhoHierarchyList, HandleItemClicked));

    SetInternalElement(this, hierarchyList_);
}

void UrhoHierarchyList::InsertItem(AbstractHierarchyListItem* item, unsigned index, AbstractHierarchyListItem* parent)
{
    auto itemWidget = MakeShared<UrhoHierarchyListItemWidget>(context_, item);
    itemWidget->SetText(item->GetText());
    itemWidget->ApplyStyle();
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
        SendEvent(E_ABSTRACTWIDGETCLICKED,
            AbstractWidgetClicked::P_ELEMENT, this,
            AbstractWidgetClicked::P_ITEM, item->GetItem());
    }
}

//////////////////////////////////////////////////////////////////////////
UrhoHierarchyListItemWidget::UrhoHierarchyListItemWidget(Context* context, AbstractHierarchyListItem* item)
    : Text(context)
    , item_(item)
{
}

void UrhoHierarchyListItemWidget::ApplyStyle()
{
    SetStyle("FileSelectorListText");
}

//////////////////////////////////////////////////////////////////////////
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
    UIElement* parent = GetParentElement(this);

    view3D_ = parent->CreateChild<View3D>();

    SetInternalElement(this, view3D_);
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

AbstractDock* UrhoMainWindow::AddDock(DockLocation hint)
{
    auto dialog = MakeShared<UrhoDock>(*this);
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
