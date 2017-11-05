#include "MenuBar.h"
#include "Common.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Window.h>

namespace Urho3D
{

extern const char* UI_CATEGORY;

static const StringHash menuTextVar("MenuText");
static const StringHash menuAccelTextVar("MenuAccelText");

MenuBar::MenuBar(Context* context)
    : BorderImage(context)
{

}

void MenuBar::RegisterObject(Context* context)
{
    context->RegisterFactory<MenuBar>(UI_CATEGORY);
    URHO3D_COPY_BASE_ATTRIBUTES(BorderImage);
}

void MenuBar::SetMenuLayout(int spacing, const IntRect& border)
{
    menuSpacing_ = spacing;
    menuBorder_ = border;
}

void MenuBar::SetPopupLayout(int spacing, const IntRect& border)
{
    popupSpacing_ = spacing;
    popupBorder_ = border;
}

Menu* MenuBar::CreateMenu(const String& text, int key, int quals, Menu* parent /*= nullptr*/)
{
    // Create popup if not exist
    if (parent != nullptr && !parent->GetPopup())
    {
        Window* popup = new Window(context_);
        popup->SetStyleAuto();
        popup->SetLayout(LM_VERTICAL, popupSpacing_, popupBorder_);
        popup->SetDefaultStyle(parent->GetDefaultStyle());
        popup->SetVisible(false);
        parent->SetPopup(popup);
        parent->SetPopupOffset(0, parent->GetHeight());

        // Add arrow for nested popups
        if (parent->GetParent() != this)
            UpdateMenuAccelText(parent, ">");
    }

    // Create menu
    Menu* menu = new Menu(context_);
    menu->SetDefaultStyle(parent->GetDefaultStyle());
    menu->SetStyleAuto();
    menu->SetLayout(LM_HORIZONTAL, menuSpacing_, menuBorder_);

    Text* menuText = menu->CreateChild<Text>();
    menuText->SetStyle("EditorMenuText");
    menuText->SetText(text);

    menu->SetVar(menuTextVar, menuText);

    // Create accelerator
    if (key != KEY_UNKNOWN)
    {
        menu->SetAccelerator(key, quals);
        UpdateMenuAccelText(menu, QualsToString(quals) + KeyToString(key));
    }

    // Add menu
    if (parent)
        parent->GetPopup()->AddChild(menu);
    else
        AddChild(menu);
    UpdateMenuSize(menu);

    return menu;
}

Menu* MenuBar::CreateMenu(const String& text, Menu* parent /*= nullptr*/)
{
    return CreateMenu(text, KEY_UNKNOWN, 0, parent);
}

Menu* MenuBar::CreateMenu(Menu* parent /*= nullptr*/)
{
    return CreateMenu(String::EMPTY, KEY_UNKNOWN, 0, parent);
}

void MenuBar::UpdateMenuAccelText(Menu* menu, const String& text)
{
    if (Text* menuText = static_cast<Text*>(menu->GetVar(menuTextVar).GetPtr()))
    {
        if (Text* accelText = static_cast<Text*>(menu->GetVar(menuAccelTextVar).GetPtr()))
        {
            accelText->SetText(text);
        }
        else
        {
            UIElement* spacer = menu->CreateChild<UIElement>();
            spacer->SetMinWidth(menuText->GetIndentSpacing());
            spacer->SetHeight(menuText->GetMinHeight());

            Text* accelKeyText = menu->CreateChild<Text>();
            accelKeyText->SetStyle("EditorMenuText");
            accelKeyText->SetTextAlignment(HA_RIGHT);
            accelKeyText->SetText(text);

            menu->SetVar(menuAccelTextVar, accelKeyText);
        }
    }
}

void MenuBar::UpdateMenuSize(Menu* menu)
{
    if (menu->GetParent() == this)
    {
        if (Text* text = static_cast<Text*>(menu->GetVar(menuTextVar).GetPtr()))
            menu->SetMaxWidth(text->GetMinWidth() + text->GetMinHeight());
    }
}

}
