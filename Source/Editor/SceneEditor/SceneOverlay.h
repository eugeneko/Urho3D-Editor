#pragma once

#include <Qt>
#include <Urho3D/Math/Ray.h>

namespace Urho3DEditor
{

/// Interface of scene overlay. Accepts some intersecting events and is able to react.
class SceneOverlay
{
public:
    /// Mouse button up/down event. Returns true if event was consumed.
    virtual bool MouseButtonEvent(const Urho3D::Ray& cameraRay, Qt::MouseButton button, bool pressed, bool consumed);
    /// General update.
    virtual void Update(const Urho3D::Ray& cameraRay, float timeStep);
    /// Post-render update. It is safe to render here.
    virtual void PostRenderUpdate(const Urho3D::Ray& cameraRay);
};

}
