#include "KeyBinding.h"
#include "AbstractInput.h"

namespace Urho3D
{

namespace
{

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
    switch (rhs)
    {
    case ModifierState::Forbidden:
        return lhs;
    case ModifierState::Optional:
        return lhs == ModifierState::Required ? ModifierState::Required : rhs;
    case ModifierState::Required:
        return ModifierState::Required;
    default:
        return ModifierState::Forbidden;
    }
}

}

const KeyBinding KeyBinding::EMPTY;
const KeyBinding KeyBinding::SHIFT(-1, -1, ModifierState::Required, ModifierState::Forbidden, ModifierState::Forbidden);
const KeyBinding KeyBinding::ALT(-1, -1, ModifierState::Forbidden, ModifierState::Required, ModifierState::Forbidden);
const KeyBinding KeyBinding::CTRL(-1, -1, ModifierState::Forbidden, ModifierState::Forbidden, ModifierState::Required);
const KeyBinding KeyBinding::OPTIONAL_SHIFT(-1, -1, ModifierState::Optional, ModifierState::Forbidden, ModifierState::Forbidden);
const KeyBinding KeyBinding::OPTIONAL_ALT(-1, -1, ModifierState::Forbidden, ModifierState::Optional, ModifierState::Forbidden);
const KeyBinding KeyBinding::OPTIONAL_CTRL(-1, -1, ModifierState::Forbidden, ModifierState::Forbidden, ModifierState::Optional);
const KeyBinding KeyBinding::ANY_MODIFIER(-1, -1, ModifierState::Optional, ModifierState::Optional, ModifierState::Optional);

const KeyBinding KeyBinding::Key(int key)
{
    return KeyBinding(-1, key, ModifierState::Forbidden, ModifierState::Forbidden, ModifierState::Forbidden);
}

const KeyBinding KeyBinding::Mouse(int mouseButton)
{
    return KeyBinding(mouseButton, -1, ModifierState::Forbidden, ModifierState::Forbidden, ModifierState::Forbidden);
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
