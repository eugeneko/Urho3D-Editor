#pragma once

#include "Editor.h"

namespace Urho3D
{

class Camera;
class CompositeKeyBinding;

/// Editor camera interface.
class CameraController : public AbstractEditorOverlay
{
    URHO3D_OBJECT(CameraController, AbstractEditorOverlay);

public:
    enum Control
    {
        MOVE_FORWARD,
        MOVE_BACK,
        MOVE_LEFT,
        MOVE_RIGHT,
        MOVE_UP,
        MOVE_DOWN,
        MOVE_ACCEL,
        TOGGLE_FLY_MODE,
        RESET_FLY_MODE,
        ROTATE,
        ORBIT,
        PAN,
        WHEEL_SCROLL_X,
        WHEEL_SCROLL_Y,
        WHEEL_SCROLL_Z,
        WHEEL_ZOOM
    };
    using Controls = HashMap<int, CompositeKeyBinding>;

    /// Construct.
    CameraController(Context* context);
    /// Set controlled camera.
    void SetCamera(Camera* camera) { camera_ = camera; }
    /// Set origin.
    //void SetOrigin(const Vector3& origin) { origin_ = origin; }

    /// Set controls.
    void SetControls(const Controls& controls);
    /// Set fly mode.
    void SetFlyMode(bool flyMode) { flyMode_ = flyMode; }
    /// Set camera position control.
    void SetPositionControl(bool controlPosition) { controlPosition_ = controlPosition; }
    /// Set speed.
    void SetSpeed(const Vector3& speed) { speed_ = speed; }
    /// Set acceleration factor.
    void SetAccelerationFactor(const Vector3& accelerationFactor) { accelerationFactor_ = accelerationFactor; }
    /// Set rotation speed.
    void SetRotationSpeed(const Vector2& rotationSpeed) { rotationSpeed_ = rotationSpeed; }
    /// Set pan speed.
    void SetPanSpeed(const Vector2& panSpeed) { panSpeed_ = panSpeed; }
    /// Set wheel speed.
    void SetWheelSpeed(const Vector3& wheelSpeed) { wheelSpeed_ = wheelSpeed; }
    /// Set default origin distance.
    void SetDefaultOriginDistance(float defaultOriginDistance) { defaultOriginDistance_ = defaultOriginDistance; }

    /// @see AbstractEditorOverlay::Update
    void Update(AbstractEditorInput& input, float timeStep) override;

private:
    /// Remove expired cameras.
    void RemoveExpired();
    /// Get camera origin.
    Vector3& GetCameraOrigin(Camera& camera);
    /// Handle current viewport changed.
    void HandleCurrentViewportChanged(StringHash eventType, VariantMap& eventData);

private:
    Camera* camera_ = nullptr;
    HashMap<WeakPtr<Camera>, Vector3> cameraOrigins_;

    Controls controls_;

    bool flyMode_ = true;
    bool controlPosition_ = true;
    Vector3 speed_ = Vector3::ONE;
    Vector3 accelerationFactor_ = Vector3::ONE;
    Vector2 rotationSpeed_ = Vector2::ONE;
    Vector2 panSpeed_ = Vector2::ONE;
    Vector3 wheelSpeed_ = Vector3::ONE;
    float defaultOriginDistance_ = 10;

    float cachedYaw_ = 0.0f;
    float cachedPitch_ = 0.0f;
    bool isMouseWrapped_ = false;
};

}
