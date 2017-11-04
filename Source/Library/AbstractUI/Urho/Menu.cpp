#include "Menu.h"
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>

namespace Urho3D
{

static const StringHash contextMenuPopup("ContextMenuPopup");

MenuWithPopupCallback::MenuWithPopupCallback(Context* context, MenuPopup* abstractMenu)
    : Menu(context)
    , menu_(abstractMenu)
{
}

void MenuWithPopupCallback::OnShowPopup()
{
    if (onShowPopup_)
        onShowPopup_();

    if (menu_)
        menu_->OnShowPopup();
}

//////////////////////////////////////////////////////////////////////////
MenuPopup::MenuPopup(Context* context, UIElement* menuRoot, const String& text)
    : Object(context)
    , menuRoot_(menuRoot)
{
    CreateMenu(menuRoot_, text);
    CreatePopup(menuRoot_);
    menu_->SetPopup(popup_);
    menu_->SetPopupOffset(0, menu_->GetHeight());
    menu_->SetMaxWidth(text_->GetMinWidth() + text_->GetMinHeight());

    menuRoot_->AddChild(menu_);
}

MenuPopup::MenuPopup(Context* context, UIElement* menuRoot, UIElement* parent, const String& text)
    : Object(context)
    , menuRoot_(menuRoot)
{
    CreateMenu(parent, text);
    CreatePopup(parent);
    menu_->SetPopup(popup_);
    menu_->SetPopupOffset(0, menu_->GetHeight());

    parent->AddChild(menu_);
}

MenuPopup::MenuPopup(Context* context)
    : Object(context)
    , menuRoot_(GetSubsystem<UI>()->GetRoot())
{
    CreatePopup(menuRoot_);
    popup_->SetVar(contextMenuPopup, true);

    menuRoot_->AddChild(popup_);
}

MenuPopup::MenuPopup(Context* context, UIElement* menuRoot, UIElement* parent, const String& text, const KeyBinding& keyBinding)
    : Object(context)
    , menuRoot_(menuRoot)
{
    CreateMenu(parent, text);
    CreateAccelerator(keyBinding);
    SubscribeToEvent(menu_, E_MENUSELECTED, URHO3D_HANDLER(MenuPopup, HandleMenuSelected));
}

void MenuPopup::OnShowPopup()
{
    if (onShowPopup_)
        onShowPopup_();
}

void MenuPopup::HandleMenuSelected(StringHash eventType, VariantMap& eventData)
{
    if (menu_->GetPopup())
        return;

    CollapsePopups();

    if (onSelected_)
        onSelected_();
}

void MenuPopup::CreatePopup(UIElement* parent)
{
    popup_ = MakeShared<Window>(context_);
    popup_->SetStyleAuto();
    popup_->SetLayout(LM_VERTICAL, 1, IntRect(2, 6, 2, 6));
    popup_->SetDefaultStyle(parent->GetDefaultStyle());
    popup_->SetVisible(false);
}

void MenuPopup::CreateMenu(UIElement* parent, const String& text)
{
    assert(!menu_ && !text_);

    menu_ = new MenuWithPopupCallback(context_, this);
    menu_->SetDefaultStyle(parent->GetDefaultStyle());
    menu_->SetStyleAuto();
    menu_->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));

    text_ = menu_->CreateChild<Text>();
    text_->SetStyle("EditorMenuText");
    text_->SetText(text);

    parent->AddChild(menu_);
}

void MenuPopup::CreateAccelerator(const KeyBinding& keyBinding)
{
    assert(menu_ && text_);

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
    accelKeyText->SetText(keyBinding.ToString());
}

void MenuPopup::CollapsePopups()
{
    Menu* menu = menu_;

    // Go to topmost menu
    while (UIElement* parent = menu->GetParent())
    {
        Menu* parentMenu = dynamic_cast<Menu*>(parent->GetVar("Origin").GetPtr());
        if (parentMenu)
            menu = parentMenu;
        else
        {
            menu->ShowPopup(false);
            if (parent->GetVar(contextMenuPopup).GetBool())
                parent->SetVisible(false);
            break;
        }
    }
}

}
