#pragma once

#include "../KeyBinding.h"
#include <Urho3D/UI/Menu.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Window.h>
#include <functional>

namespace Urho3D
{

class MenuPopup;

class MenuWithPopupCallback : public Menu
{
public:
    std::function<void()> onShowPopup_;

public:
    MenuWithPopupCallback(Context* context, MenuPopup* menu);

    virtual void OnShowPopup() override;

private:
    MenuPopup* menu_ = nullptr;
};

class MenuPopup : public Object
{
    URHO3D_OBJECT(MenuPopup, Object);

public:
    std::function<void()> onShowPopup_;
    std::function<void()> onSelected_;

public:
    /// Create top-level menu.
    MenuPopup(Context* context, UIElement* menuRoot, const String& text);
    /// Create menu with popup.
    MenuPopup(Context* context, UIElement* menuRoot, UIElement* parent, const String& text);
    /// Create context menu popup.
    MenuPopup(Context* context);
    /// Create action menu.
    MenuPopup(Context* context, UIElement* menuRoot, UIElement* parent, const String& text, const KeyBinding& keyBinding);

    /// Called when popup is shown.
    void OnShowPopup();

    /// Get menu root.
    UIElement* GetMenuRoot() const { return menuRoot_; }
    /// Get popup element.
    Window* GetPopupElement() const { return popup_; }
    /// Get text element.
    Text* GetTextElement() const { return text_; }

private:
    void HandleMenuSelected(StringHash eventType, VariantMap& eventData);

    void CreatePopup(UIElement* parent);
    void CreateMenu(UIElement* parent, const String& text);
    void CreateAccelerator(const KeyBinding& keyBinding);
    void CollapsePopups();

private:
    UIElement* menuRoot_ = nullptr;
    Menu* menu_ = nullptr;
    Text* text_ = nullptr;
    SharedPtr<Window> popup_ = nullptr;

};

}
