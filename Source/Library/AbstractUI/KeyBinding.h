#pragma once

#include <Urho3D/Container/Vector.h>
#include <initializer_list>

namespace Urho3D
{

class AbstractInput;

/// Modifier state.
enum class ModifierState
{
    Forbidden,
    Optional,
    Required
};

/// Key binding.
class KeyBinding
{
public:
    static const KeyBinding EMPTY;
    static const KeyBinding SHIFT;
    static const KeyBinding ALT;
    static const KeyBinding CTRL;
    static const KeyBinding NO_SHIFT;
    static const KeyBinding NO_ALT;
    static const KeyBinding NO_CTRL;
    static const KeyBinding Key(int key);
    static const KeyBinding Mouse(int mouseButton);
    KeyBinding() {}
    KeyBinding(int mouseButton, int key, ModifierState shift, ModifierState alt, ModifierState ctrl);
    friend KeyBinding operator +(KeyBinding lhs, const KeyBinding& rhs);
    bool IsDown(AbstractInput& input, bool ignoreGrabbed = true, bool grab = true) const;
    bool IsPressed(AbstractInput& input, bool ignoreGrabbed = true, bool grab = true) const;

    bool IsEmpty() const { return key_ < 0 && mouseButton_ < 0; }
    int GetKey() const { return key_; }
    ModifierState GetShift() const { return shift_; }
    ModifierState GetAlt() const { return alt_; }
    ModifierState GetCtrl() const { return ctrl_; }
private:
    int mouseButton_ = -1;
    int key_ = -1;
    ModifierState shift_ = ModifierState::Optional;
    ModifierState alt_ = ModifierState::Optional;
    ModifierState ctrl_ = ModifierState::Optional;
};

/// Composite key binding.
class CompositeKeyBinding
{
public:
    CompositeKeyBinding() {}
    CompositeKeyBinding(const KeyBinding& keyBinding) : keyBindings_{ keyBinding } {}
    CompositeKeyBinding(const std::initializer_list<KeyBinding>& keyBindings) : keyBindings_(keyBindings) {}
    bool IsDown(AbstractInput& input, bool ignoreGrabbed = true, bool grab = true) const;
    bool IsPressed(AbstractInput& input, bool ignoreGrabbed = true, bool grab = true) const;
private:
    Vector<KeyBinding> keyBindings_;
};

}
