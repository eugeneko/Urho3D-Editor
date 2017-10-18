#pragma once

#include <Urho3D/Math/Ray.h>
#include <Urho3D/Input/Input.h>

namespace Urho3D
{

class UI;
class Camera;
class AbstractInput;

/// Interface of editor context.
class AbstractEditorContext : public Object
{
    URHO3D_OBJECT(AbstractEditorContext, Object);

public:
    /// Construct.
    AbstractEditorContext(Context* context) : Object(context) {}

    /// Return mouse ray in 3D.
    virtual Ray GetMouseRay() const = 0;
    /// Return current camera.
    virtual Camera* GetCurrentCamera() const = 0;
};

/// Interface of editor overlay.
class AbstractEditorOverlay : public Object
{
    URHO3D_OBJECT(AbstractEditorOverlay, Object);

public:
    /// Construct.
    AbstractEditorOverlay(Context* context) : Object(context) {}
    /// General update.
    virtual void Update(AbstractInput& input, AbstractEditorContext& editorContext, float timeStep);
    /// Post-render update. It is safe to render here.
    virtual void PostRenderUpdate(AbstractInput& input, AbstractEditorContext& editorContext);
};

}
