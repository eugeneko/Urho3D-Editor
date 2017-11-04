#include "KeyBinding.h"
#include "AbstractInput.h"

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

/// Test modifier state
bool TestModifier(bool isKeyDown, ModifierState state)
{
    switch (state)
    {
    case ModifierState::Forbidden:
        return !isKeyDown;
    case ModifierState::Optional:
        return true;
    case ModifierState::Required:
        return isKeyDown;
    default:
        return false;
    }
}

/// Merge modifier.
ModifierState MergeModifier(ModifierState lhs, ModifierState rhs)
{
    if (lhs == ModifierState::Required || rhs == ModifierState::Required)
        return ModifierState::Required;
    else if (lhs == ModifierState::Forbidden || rhs == ModifierState::Forbidden)
        return ModifierState::Forbidden;
    else
        return ModifierState::Optional;
}

}

const KeyBinding KeyBinding::EMPTY;
const KeyBinding KeyBinding::SHIFT(-1, -1, ModifierState::Required, ModifierState::Optional, ModifierState::Optional);
const KeyBinding KeyBinding::ALT(-1, -1, ModifierState::Optional, ModifierState::Required, ModifierState::Optional);
const KeyBinding KeyBinding::CTRL(-1, -1, ModifierState::Optional, ModifierState::Optional, ModifierState::Required);
const KeyBinding KeyBinding::NO_SHIFT(-1, -1, ModifierState::Forbidden, ModifierState::Optional, ModifierState::Optional);
const KeyBinding KeyBinding::NO_ALT(-1, -1, ModifierState::Optional, ModifierState::Forbidden, ModifierState::Optional);
const KeyBinding KeyBinding::NO_CTRL(-1, -1, ModifierState::Optional, ModifierState::Optional, ModifierState::Forbidden);

const KeyBinding KeyBinding::Key(int key)
{
    return KeyBinding(-1, key, ModifierState::Optional, ModifierState::Optional, ModifierState::Optional);
}

const KeyBinding KeyBinding::Mouse(int mouseButton)
{
    return KeyBinding(mouseButton, -1, ModifierState::Optional, ModifierState::Optional, ModifierState::Optional);
}

KeyBinding::KeyBinding(int mouseButton, int key, ModifierState shift, ModifierState alt, ModifierState ctrl)
    : mouseButton_(mouseButton)
    , key_(key)
    , shift_(shift)
    , alt_(alt)
    , ctrl_(ctrl)
{
}

bool KeyBinding::IsDown(AbstractInput& input, bool ignoreGrabbed, bool grab) const
{
    // Check all conditions
    if (mouseButton_ >= 0)
    {
        if (!input.IsMouseButtonDown(mouseButton_))
            return false;
        if (ignoreGrabbed && input.IsMouseButtonGrabbed(mouseButton_))
            return false;
    }
    if (key_ >= 0)
    {
        if (!input.IsKeyDown(key_))
            return false;
        if (ignoreGrabbed && input.IsKeyGrabbed(key_))
            return false;
    }
    if (!TestModifier(input.IsKeyDown(KEY_SHIFT), shift_))
        return false;
    if (!TestModifier(input.IsKeyDown(KEY_ALT), alt_))
        return false;
    if (!TestModifier(input.IsKeyDown(KEY_CTRL), ctrl_))
        return false;

    // Grab if needed
    if (grab)
    {
        if (mouseButton_ >= 0)
            input.GrabMouseButton(mouseButton_);
        if (key_ >= 0)
            input.GrabKey(key_);
    }
    return true;
}

bool KeyBinding::IsPressed(AbstractInput& input, bool ignoreGrabbed, bool grab) const
{
    // Check all conditions
    if (mouseButton_ >= 0)
    {
        if (!input.IsMouseButtonPressed(mouseButton_))
            return false;
        if (ignoreGrabbed && input.IsMouseButtonGrabbed(mouseButton_))
            return false;
    }
    if (key_ >= 0)
    {
        if (!input.IsKeyPressed(key_))
            return false;
        if (ignoreGrabbed && input.IsKeyGrabbed(key_))
            return false;
    }
    if (!TestModifier(input.IsKeyDown(KEY_SHIFT), shift_))
        return false;
    if (!TestModifier(input.IsKeyDown(KEY_ALT), alt_))
        return false;
    if (!TestModifier(input.IsKeyDown(KEY_CTRL), ctrl_))
        return false;

    // Grab if needed
    if (grab)
    {
        if (mouseButton_ >= 0)
            input.GrabMouseButton(mouseButton_);
        if (key_ >= 0)
            input.GrabKey(key_);
    }
    return true;
}

String KeyBinding::ToString() const
{
    String result;
    if (GetShift() == ModifierState::Required)
        result += "Shift+";
    if (GetCtrl() == ModifierState::Required)
        result += "Ctrl+";
    if (GetAlt() == ModifierState::Required)
        result += "Alt+";
    result += PrintKey(GetKey());
    return result;
}

KeyBinding operator+(KeyBinding lhs, const KeyBinding& rhs)
{
    if (lhs.mouseButton_ < 0 && rhs.mouseButton_ >= 0)
        lhs.mouseButton_ = rhs.mouseButton_;
    if (lhs.key_ < 0 && rhs.key_ >= 0)
        lhs.key_ = rhs.key_;
    lhs.shift_ = MergeModifier(lhs.shift_, rhs.shift_);
    lhs.alt_ = MergeModifier(lhs.alt_, rhs.alt_);
    lhs.ctrl_ = MergeModifier(lhs.ctrl_, rhs.ctrl_);
    return lhs;
}

//////////////////////////////////////////////////////////////////////////
bool CompositeKeyBinding::IsDown(AbstractInput& input, bool ignoreGrabbed /*= true*/, bool grab /*= true*/) const
{
    for (const KeyBinding& keyBinding : keyBindings_)
        if (keyBinding.IsDown(input, ignoreGrabbed, grab))
            return true;
    return false;
}

bool CompositeKeyBinding::IsPressed(AbstractInput& input, bool ignoreGrabbed /*= true*/, bool grab /*= true*/) const
{
    for (const KeyBinding& keyBinding : keyBindings_)
        if (keyBinding.IsPressed(input, ignoreGrabbed, grab))
            return true;
    return false;
}

}
