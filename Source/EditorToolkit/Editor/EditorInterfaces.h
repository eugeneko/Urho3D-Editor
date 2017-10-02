#pragma once

#include <Urho3D/Math/Ray.h>
#include <Urho3D/Input/Input.h>

namespace Urho3D
{

class UI;

/// Interface of editor input provider.
class AbstractEditorInput : public Object
{
    URHO3D_OBJECT(AbstractEditorInput, Object);

public:
    /// Construct.
    AbstractEditorInput(Context* context) : Object(context) {}
    /// Reset grab.
    virtual void ResetGrab() = 0;

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
    /// Return mouse ray in 3D.
    virtual Ray GetMouseRay() const = 0;
    /// Return far clip distance.
    virtual float GetFarClip() const = 0;

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

/// Interface of editor overlay.
class AbstractEditorOverlay : public Object
{
    URHO3D_OBJECT(AbstractEditorOverlay, Object);

public:
    /// Construct.
    AbstractEditorOverlay(Context* context) : Object(context) {}
    /// General update.
    virtual void Update(AbstractEditorInput& input, float timeStep);
    /// Post-render update. It is safe to render here.
    virtual void PostRenderUpdate(AbstractEditorInput& input);
};

}
