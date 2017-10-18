#pragma once

#include <Urho3D/Container/HashSet.h>
#include <Urho3D/Input/Input.h>

namespace Urho3D
{

class AbstractInput
{
public:
    /// Set mouse mode.
    virtual void SetMouseMode(MouseMode mouseMode) = 0;
    /// Return whether the UI is focused.
    virtual bool IsUIFocused() const = 0;
    /// Return whether the UI is hovered.
    virtual bool IsUIHovered() const = 0;

    /// Return whether the key is down.
    virtual bool IsKeyDown(int key) const = 0;
    /// Return whether the key is pressed.
    virtual bool IsKeyPressed(int key) const = 0;
    /// Return whether the mouse button is down.
    virtual bool IsMouseButtonDown(int mouseButton) const = 0;
    /// Return whether the mouse button is pressed.
    virtual bool IsMouseButtonPressed(int mouseButton) const = 0;
    /// Return mouse position.
    virtual IntVector2 GetMousePosition() const = 0;
    /// Return mouse move.
    virtual IntVector2 GetMouseMove() const = 0;
    /// Return mouse wheel delta.
    virtual int GetMouseWheelMove() const = 0;

    /// Reset grab.
    virtual void ResetGrab() = 0;
    /// Grab key.
    virtual void GrabKey(int key) = 0;
    /// Checks whether key is grabbed.
    virtual bool IsKeyGrabbed(int key) const = 0;
    /// Grab mouse button input.
    virtual void GrabMouseButton(int mouseButton) = 0;
    /// Checks whether mouse button is grabbed.
    virtual bool IsMouseButtonGrabbed(int mouseButton) const = 0;
    /// Grab mouse button input.
    virtual void GrabMouseMove() = 0;
    /// Checks whether mouse button is grabbed.
    virtual bool IsMouseMoveGrabbed() const = 0;
};

class StandardInput : public AbstractInput
{
public:
    /// \see AbstractInput::GetFarClip
    void ResetGrab() override;

    /// \see AbstractInput::GrabKey
    void GrabKey(int key) override;
    /// \see AbstractInput::IsKeyGrabbed
    bool IsKeyGrabbed(int key) const override;
    /// \see AbstractInput::GrabMouseButton
    void GrabMouseButton(int mouseButton) override;
    /// \see AbstractInput::IsMouseButtonGrabbed
    bool IsMouseButtonGrabbed(int mouseButton) const override;
    /// \see AbstractInput::GrabMouseMove
    void GrabMouseMove() override;
    /// \see AbstractInput::IsMouseMoveGrabbed
    bool IsMouseMoveGrabbed() const override;

private:
    /// Grabbed keys.
    HashSet<int> grabbedKeys_;
    /// Grabbed mouse buttons.
    HashSet<int> grabbedMouseButtons_;
    /// Whether the mouse movement is grabbed.
    bool grabbedMouseMove_ = false;

};

}
