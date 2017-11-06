#include "TabBar.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UIEvents.h>

namespace Urho3D
{

extern const char* UI_CATEGORY;

TabBar::TabBar(Context* context)
    : BorderImage(context)
{
    SetEnabled(true);
    SetFocusMode(FM_FOCUSABLE);
    SetLayout(LM_HORIZONTAL, 8);
}

void TabBar::RegisterObject(Context* context)
{
    context->RegisterFactory<TabBar>(UI_CATEGORY);
    URHO3D_COPY_BASE_ATTRIBUTES(BorderImage);
}

void TabBar::SetTabBorder(const IntRect& tabBorder)
{
    tabBorder_ = tabBorder;
}

void TabBar::SetScrollSpeed(int scrollSpeed)
{
    scrollSpeed_ = scrollSpeed;
}

Button* TabBar::AddTab(const String& text)
{
    Button* tabButton = new Button(context_);
    tabButton->SetDefaultStyle(GetDefaultStyle());
    tabButton->SetStyle("Menu");
    tabButton->SetLayout(LM_HORIZONTAL, 0, tabBorder_);
    tabButton->SetFocusMode(FM_RESETFOCUS);

    Text* tabText = tabButton->CreateChild<Text>();
    tabText->SetStyle("EditorMenuText");
    tabText->SetText(text);
    tabText->SetTextAlignment(HA_CENTER);

    tabButton->SetFixedWidth(tabText->GetMinWidth() + tabText->GetMinHeight());
    SubscribeToEvent(tabButton, E_PRESSED, URHO3D_HANDLER(TabBar, HandleTabClicked));

    // Select if first added tab
    if (tabs_.Empty())
        tabButton->SetSelected(true);

    tabs_.Push(tabButton);
    AddChild(tabButton);

    UpdateOffset();

    return tabButton;
}

void TabBar::OnResize(const IntVector2& newSize, const IntVector2& delta)
{
    UpdateOffset();
}

void TabBar::OnWheel(int delta, int /*buttons*/, int /*qualifiers*/)
{
    offset_ += delta * scrollSpeed_;
    UpdateOffset();
}

void TabBar::HandleTabClicked(StringHash /*eventType*/, VariantMap& eventData)
{
    Button* button = static_cast<Button*>(eventData[Pressed::P_ELEMENT].GetPtr());
    for (Button* tab : tabs_)
        tab->SetSelected(button == tab);
}

void TabBar::UpdateOffset()
{
    const int maxOffset = Max(0, GetEffectiveMinSize().x_ - GetMaxWidth());
    offset_ = Clamp(offset_, 0, maxOffset);
    SetChildOffset(IntVector2(-offset_, 0));
}

}
