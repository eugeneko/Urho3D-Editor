#include "TabBar.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UIEvents.h>

namespace Urho3D
{

extern const char* UI_CATEGORY;

TabButton::TabButton(Context* context)
    : Button(context)
{
}

void TabButton::RegisterObject(Context* context)
{
    context->RegisterFactory<TabButton>(UI_CATEGORY);
    URHO3D_COPY_BASE_ATTRIBUTES(Button);
}

void TabButton::Update(float timeStep)
{
    Button::Update(timeStep);

    // Enforce hovering if dragging
    if (isDragging_)
        hovering_ = true;
}

void TabButton::OnDragBegin(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor)
{
    Button::OnDragBegin(position, screenPosition, buttons, qualifiers, cursor);
    if (buttons & MOUSEB_LEFT)
    {
        isDragging_ = true;
    }
}

void TabButton::OnDragEnd(const IntVector2& position, const IntVector2& screenPosition, int dragButtons, int buttons, Cursor* cursor)
{
    Button::OnDragEnd(position, screenPosition, dragButtons, buttons, cursor);
    if (isDragging_)
    {
        isDragging_ = false;
//         SetPressed(false);
    }
}

void TabButton::OnDragCancel(const IntVector2& position, const IntVector2& screenPosition, int dragButtons, int buttons, Cursor* cursor)
{
    Button::OnDragCancel(position, screenPosition, dragButtons, buttons, cursor);
    if (isDragging_)
    {
        isDragging_ = false;
//         SetPressed(false);
    }
}

//////////////////////////////////////////////////////////////////////////
TabBar::TabBar(Context* context)
    : BorderImage(context)
{
    SetEnabled(true);
    SetFocusMode(FM_FOCUSABLE);
    SetLayout(LM_HORIZONTAL, 8);

    // Add filler
    filler_ = CreateChild<UIElement>();
    filler_->SetVisible(false);
}

void TabBar::RegisterObject(Context* context)
{
    context->RegisterFactory<TabBar>(UI_CATEGORY);
    URHO3D_COPY_BASE_ATTRIBUTES(BorderImage);
}

void TabBar::SetFill(bool fill)
{
    filler_->SetVisible(fill);
}

void TabBar::SetTabBorder(const IntRect& tabBorder)
{
    tabBorder_ = tabBorder;
}

void TabBar::SetScrollSpeed(int scrollSpeed)
{
    scrollSpeed_ = scrollSpeed;
}

TabButton* TabBar::AddTab(const String& text)
{
    TabButton* tabButton = new TabButton(context_);
    tabButton->SetDefaultStyle(GetDefaultStyle());
    tabButton->SetStyle("Menu");
    tabButton->SetLayout(LM_HORIZONTAL, 0, tabBorder_);
    tabButton->SetFocusMode(FM_RESETFOCUS);

    Text* tabText = tabButton->CreateChild<Text>();
    tabText->SetStyle("EditorMenuText");
    tabText->SetText(text);
    tabText->SetTextAlignment(HA_CENTER);

    tabButton->SetFixedWidth(tabText->GetMinWidth() + tabText->GetMinHeight());
    SubscribeToEvent(tabButton, E_ITEMCLICKED, URHO3D_HANDLER(TabBar, HandleTabClicked));

    SubscribeToEvent(tabButton, E_DRAGBEGIN,
        [=](StringHash eventType, VariantMap& eventData)
    {
        dragBeginIndex_ = tabs_.IndexOf(tabButton);
    });

    SubscribeToEvent(tabButton, E_DRAGMOVE,
        [=](StringHash eventType, VariantMap& eventData)
    {
        const unsigned movingButtonIndex = tabs_.IndexOf(tabButton);
        const IntVector2 screenPos(eventData[DragMove::P_X].GetInt(), eventData[DragMove::P_Y].GetInt());
        assert(movingButtonIndex < tabs_.Size());
        for (unsigned i = 0; i < movingButtonIndex; ++i)
        {
            const int referenceX = tabs_[i]->GetScreenPosition().x_ + tabs_[i]->GetWidth() / 4;
            if (referenceX > screenPos.x_)
            {
                ReorderTab(tabButton, i);
                break;
            }
        }
        for (unsigned i = tabs_.Size(); i > movingButtonIndex + 1; --i)
        {
            const int referenceX = tabs_[i - 1]->GetScreenPosition().x_ + tabs_[i - 1]->GetWidth() * 3 / 4;
            if (referenceX < screenPos.x_)
            {
                ReorderTab(tabButton, i - 1);
                break;
            }
        }
    });

    SubscribeToEvent(tabButton, E_DRAGCANCEL,
        [=](StringHash eventType, VariantMap& eventData)
    {
        ReorderTab(tabButton, dragBeginIndex_);
    });

    // Select if first added tab
    if (tabs_.Empty())
        tabButton->SetSelected(true);

    // Insert new tab before filler
    tabs_.Push(tabButton);
    assert(GetNumChildren() > 0);
    InsertChild(GetNumChildren() - 1, tabButton);

    // Update layout
    UpdateOffset();

    return tabButton;
}

void TabBar::ReorderTab(TabButton* tab, unsigned index)
{
    const unsigned oldIndex = tabs_.IndexOf(tab);
    assert(oldIndex < tabs_.Size());

    if (oldIndex == index)
        return;

    SharedPtr<TabButton> tabHolder(tab);
    RemoveChildAtIndex(oldIndex);
    tabs_.Erase(oldIndex);

    InsertChild(index, tab);
    tabs_.Insert(index, tab);
}

void TabBar::OnResize(const IntVector2& newSize, const IntVector2& delta)
{
    UpdateOffset();
}

void TabBar::OnWheel(int delta, int /*buttons*/, int /*qualifiers*/)
{
    offset_ -= delta * scrollSpeed_;
    UpdateOffset();
}

void TabBar::HandleTabClicked(StringHash /*eventType*/, VariantMap& eventData)
{
    TabButton* button = static_cast<TabButton*>(eventData[Pressed::P_ELEMENT].GetPtr());
    for (TabButton* tab : tabs_)
        tab->SetSelected(button == tab);
}

void TabBar::UpdateOffset()
{
    const int maxOffset = Max(0, GetEffectiveMinSize().x_ - GetParent()->GetWidth());
    offset_ = Clamp(offset_, 0, maxOffset);
    SetChildOffset(IntVector2(-offset_, 0));
}

}