#include "CameraController.h"
#include "EditorViewportLayout.h"
#include "EditorEvents.h"
#include "../AbstractUI/AbstractInput.h"
#include "../AbstractUI/KeyBinding.h"

namespace Urho3D
{

CameraController::CameraController(Context* context)
    : AbstractEditorOverlay(context)
{
    // \todo Use more specific things
    SubscribeToEvent(E_EDITORCURRENTVIEWPORTCHANGED, URHO3D_HANDLER(CameraController, HandleCurrentViewportChanged));
}

void CameraController::SetControls(const Controls& controls)
{
    controls_ = controls;
}

void CameraController::Update(AbstractInput& input, AbstractEditorContext& editorContext, float timeStep)
{
    if (!camera_ || input.IsUIFocused())
        return;

    RemoveExpired();
    Node& node = *camera_->GetNode();
    Vector3& origin = GetCameraOrigin(*camera_);

    // Update fly mode
    if (controls_[TOGGLE_FLY_MODE].IsPressed(input))
        flyMode_ = !flyMode_;

    // Move camera.
    if (flyMode_ || controlPosition_)
    {
        const bool isAccelerated = controls_[MOVE_ACCEL].IsDown(input);
        const Vector3 movementSpeed = isAccelerated ? speed_ * accelerationFactor_ : speed_;
        const Vector3 oldPosition = node.GetWorldPosition();

        if (controls_[MOVE_FORWARD].IsDown(input))
            node.Translate(Vector3::FORWARD * movementSpeed * timeStep);
        if (controls_[MOVE_BACK].IsDown(input))
            node.Translate(Vector3::BACK * movementSpeed * timeStep);
        if (controls_[MOVE_LEFT].IsDown(input))
            node.Translate(Vector3::LEFT * movementSpeed * timeStep);
        if (controls_[MOVE_RIGHT].IsDown(input))
            node.Translate(Vector3::RIGHT * movementSpeed * timeStep);
        if (controls_[MOVE_UP].IsDown(input))
            node.Translate(Vector3::UP * movementSpeed * timeStep, TS_WORLD);
        if (controls_[MOVE_DOWN].IsDown(input))
            node.Translate(Vector3::DOWN * movementSpeed * timeStep, TS_WORLD);

        // Update origin
        origin += node.GetWorldPosition() - oldPosition;
    }

    // Apply mouse wheel
    const float wheelDelta = static_cast<float>(input.GetMouseWheelMove());
    if (wheelDelta != 0 && !input.IsUIHovered())
    {
        // Apply zoom
        if (controls_[WHEEL_ZOOM].IsDown(input))
        {
            const float zoom = camera_->GetZoom() + -wheelDelta * 0.1f;
            camera_->SetZoom(Clamp(zoom, 0.1f, 30.0f));
        }

        // Apply Z scroll
        if (controls_[WHEEL_SCROLL_Z].IsDown(input))
        {
            if (camera_->IsOrthographic())
            {
                node.Translate(wheelSpeed_ * Vector3::FORWARD * wheelDelta);
            }
            else
            {
                const float distance = (node.GetWorldPosition() - origin).Length();
                const float ratio = distance / 40.0f;
                const float factor = ratio < 1.0f ? ratio : 1.0f;
                node.Translate(wheelSpeed_ * Vector3::FORWARD * wheelDelta * factor);
            }
        }

        // Apply X scroll
        if (controls_[WHEEL_SCROLL_X].IsDown(input))
        {
            const Vector3 oldPosition = node.GetWorldPosition();
            node.Translate(wheelSpeed_ * Vector3::RIGHT * wheelDelta);
            origin += node.GetWorldPosition() - oldPosition;
        }

        // Apply Y scroll
        if (controls_[WHEEL_SCROLL_Y].IsDown(input))
        {
            const Vector3 oldPosition = node.GetWorldPosition();
            node.Translate(wheelSpeed_ * Vector3::UP * wheelDelta);
            origin += node.GetWorldPosition() - oldPosition;
        }
    }

    // Rotate camera
    bool wrapMouse = false;

    const IntVector2 mouseMove = input.GetMouseMove();
    const bool isRotating = flyMode_ || controls_[ROTATE].IsDown(input);
    const bool isOrbiting = !flyMode_ && controls_[ORBIT].IsDown(input);
    if (isRotating || isOrbiting)
    {
        const Vector3& oldDirection = node.GetWorldDirection();
        const float yaw = Atan2(oldDirection.z_, oldDirection.x_);
        const float pitch = Asin(oldDirection.y_);
        if (Abs(cachedYaw_ - yaw) > M_LARGE_EPSILON)
            cachedYaw_ = yaw;
        if (Abs(cachedPitch_ - pitch) > M_LARGE_EPSILON)
            cachedPitch_ = pitch;

        cachedYaw_ -= mouseMove.x_ * rotationSpeed_.x_;
        cachedPitch_ -= mouseMove.y_ * rotationSpeed_.y_;
        cachedPitch_ = Urho3D::Clamp(cachedPitch_, -89.0f, 89.0f);

        const Vector3 newDirection(Cos(cachedYaw_) * Cos(cachedPitch_), Sin(cachedPitch_), Sin(cachedYaw_) * Cos(cachedPitch_));
        node.LookAt(node.GetWorldPosition() + newDirection);
        wrapMouse = true;
    }

    // Orbit camera
    if (isOrbiting)
    {
        const Vector3 delta = node.GetWorldPosition() - origin;
        node.SetWorldPosition(origin - node.GetWorldRotation() * Vector3(0.0, 0.0, delta.Length()));
    }

    // Pan camera
    const bool isPanning = !flyMode_ && controls_[PAN].IsDown(input);
    if (isPanning)
    {
        const Vector3 floatMouseMove = Vector3(static_cast<float>(-mouseMove.x_), static_cast<float>(mouseMove.y_), 0.0f);
        const Vector3 delta = floatMouseMove * timeStep * Vector3(panSpeed_, 0.0f);
        const Vector3 oldPosition = node.GetWorldPosition();
        node.Translate(delta);

        // Update origin
        origin += node.GetWorldPosition() - oldPosition;
    }

    // Update mouse state
    if (wrapMouse && !isMouseWrapped_)
    {
        isMouseWrapped_ = true;
        input.SetMouseMode(MM_WRAP);
    }
    else if (!wrapMouse && isMouseWrapped_)
    {
        isMouseWrapped_ = false;
        input.SetMouseMode(MM_ABSOLUTE);
    }

    // TODO: Implement 'Go Home'
    // TODO: Implement 'View Closer'
}

void CameraController::RemoveExpired()
{
    for (auto iter = cameraOrigins_.Begin(); iter != cameraOrigins_.End(); )
    {
        if (iter->first_.Expired())
            iter = cameraOrigins_.Erase(iter);
        else
            ++iter;
    }
}

Vector3& CameraController::GetCameraOrigin(Camera& camera)
{
    WeakPtr<Camera> weakCamera(&camera);
    auto iter = cameraOrigins_.Find(weakCamera);
    if (iter == cameraOrigins_.End())
    {
        Node& cameraNode = *camera.GetNode();
        const Vector3 defaultOrigin = cameraNode.GetWorldPosition() + cameraNode.GetWorldDirection() * defaultOriginDistance_;
        iter = cameraOrigins_.Insert(MakePair(weakCamera, defaultOrigin));
    }
    return iter->second_;
}

void CameraController::HandleCurrentViewportChanged(StringHash eventType, VariantMap& eventData)
{
    Camera* newCamera = dynamic_cast<Camera*>(eventData[EditorCurrentViewportChanged::P_CAMERA].GetPtr());
    SetCamera(newCamera);
}

}
