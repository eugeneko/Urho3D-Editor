#pragma once

#include <Qt>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Input/Input.h>

namespace Urho3DEditor
{

/// Input interface of scene overlay.
class SceneInputInterface
{
public:
    /// Set mouse mode.
    virtual void SetMouseMode(Urho3D::MouseMode mouseMode) = 0;
    /// Return whether the key is down.
    virtual bool IsKeyDown(Qt::Key key) const = 0;
    /// Return whether the key is pressed.
    virtual bool IsKeyPressed(Qt::Key key) const = 0;
    /// Return whether the mouse button is down.
    virtual bool IsMouseButtonDown(Qt::MouseButton mouseButton) const = 0;
    /// Return whether the mouse button is pressed.
    virtual bool IsMouseButtonPressed(Qt::MouseButton mouseButton) const = 0;
    /// Return mouse position.
    virtual Urho3D::IntVector2 GetMousePosition() const = 0;
    /// Return mouse move.
    virtual Urho3D::IntVector2 GetMouseMove() const = 0;
    /// Return mouse wheel delta.
    virtual int GetMouseWheelMove() const = 0;
    /// Return mouse ray in 3D.
    virtual Urho3D::Ray GetMouseRay() const = 0;

};

/// Interface of scene overlay. Accepts some intersecting events and is able to react.
class SceneOverlay
{
public:
    /// General update.
    virtual void Update(SceneInputInterface& input, float timeStep);
    /// Post-render update. It is safe to render here.
    virtual void PostRenderUpdate(SceneInputInterface& input);
};

}
