#include "CameraController.h"
#include "KeyBinding.h"
#include "EditorViewportLayout.h"

namespace Urho3D
{

CameraController::CameraController(Context* context)
    : AbstractEditorOverlay(context)
{
    SubscribeToEvent(E_EDITORCURRENTVIEWPORTCHANGED, URHO3D_HANDLER(CameraController, HandleCurrentViewportChanged));
}

void CameraController::SetControls(const Controls& controls)
{
    controls_ = controls;
}

void CameraController::Update(AbstractEditorInput& input, float timeStep)
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

#if 0
    if (input.GetMouseWheelMove() != 0)
    {
        if (hotKeyMode == HotKeyMode::Standard)
        {
            if (mouseWheelCameraPosition)
            {
                cameraNode_.Translate(Vector3(0, 0, -cameraBaseSpeed) * -input.GetMouseWheelMove() * 20 * timeStep *
                    speedMultiplier);
            }
            else
            {
                const float zoom = camera_.GetZoom() + -input.GetMouseWheelMove() * 0.1 * speedMultiplier;
                camera_.SetZoom(Clamp(zoom, 0.1f, 30.0f));
            }
        }
        else if (hotKeyMode == HotKeyMode::Blender)
        {
            if (mouseWheelCameraPosition && !camera_.IsOrthographic())
            {
                if (input.IsKeyDown(Qt::Key_Shift))
                    cameraNode_.Translate(Vector3(0, -cameraBaseSpeed, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                else if (input.IsKeyDown(Qt::Key_Control))
                    cameraNode_.Translate(Vector3(-cameraBaseSpeed, 0, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                else
                {
                    float distance = (cameraNode_.GetWorldPosition() - p.selectionCenter_).Length();
                    float ratio = distance / 40.0f;
                    float factor = ratio < 1.0f ? ratio : 1.0f;
                    cameraNode_.Translate(Vector3(0, 0, -cameraBaseSpeed) * -input.GetMouseWheelMove() * 40 * factor*timeStep*speedMultiplier);
                }
            }
            else
            {
                if (input.IsKeyDown(Qt::Key_Shift))
                {
                    cameraNode_.Translate(Vector3(0, -cameraBaseSpeed, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                }
                else if (input.IsKeyDown(Qt::Key_Control))
                {
                    cameraNode_.Translate(Vector3(-cameraBaseSpeed, 0, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                }
                else
                {
                    if (input.IsKeyDown(Qt::Key_Alt))
                    {
                        const float zoom = camera_.GetZoom() + -input.GetMouseWheelMove() * 0.1f * speedMultiplier;
                        camera_.SetZoom(Clamp(zoom, 0.1f, 30.0f));
                    }
                    else
                    {
                        cameraNode_.Translate(Vector3(0, 0, -cameraBaseSpeed) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                    }
                }
            }
        }
    }

    if (input.IsKeyDown(Qt::Key_Home))
    {
        if (p.hasSelection_)
        {
            Vector3 d = cameraNode_.GetWorldPosition() - p.selectionCenter_;
            cameraNode_.SetWorldPosition(p.selectionCenter_ - cameraNode_.GetRotation() * Vector3(0.0, 0.0, 10));
        }
    }

    // Rotate/orbit/pan camera
    bool changeCamViewButton = false;

    if (hotKeyMode == HotKeyMode::Standard)
        changeCamViewButton = input.IsMouseButtonDown(Qt::RightButton) || input.IsMouseButtonDown(Qt::MiddleButton);
    else if (hotKeyMode == HotKeyMode::Blender)
    {
        changeCamViewButton = input.IsMouseButtonDown(Qt::MiddleButton) || flyMode_;

        if (input.IsMouseButtonDown(Qt::RightButton) || input.IsKeyDown(Qt::Key_Escape))
            flyMode_ = false;
    }

    if (changeCamViewButton)
    {
        input.SetMouseMode(MM_WRAP);

        const IntVector2 mouseMove = input.GetMouseMove();
        if (mouseMove.x_ != 0 || mouseMove.y_ != 0)
        {
            bool panTheCamera = false;

            if (hotKeyMode == HotKeyMode::Standard)
            {
                if (input.IsMouseButtonDown(Qt::MiddleButton))
                {
                    if (mmbPanMode)
                        panTheCamera = !input.IsKeyDown(Qt::Key_Shift);
                    else
                        panTheCamera = input.IsKeyDown(Qt::Key_Shift);
                }
            }
            else if (hotKeyMode == HotKeyMode::Blender)
            {
                if (!flyMode_)
                    panTheCamera = input.IsKeyDown(Qt::Key_Shift);
            }

            if (panTheCamera)
                cameraNode_.Translate(Vector3(-mouseMove.x_, mouseMove.y_, 0) * timeStep * cameraBaseSpeed * 0.5);
            else
            {
                cameraAngles_.x_ += mouseMove.y_ * cameraBaseRotationSpeed;
                cameraAngles_.y_ += mouseMove.x_ * cameraBaseRotationSpeed;

                if (limitRotation)
                    cameraAngles_.x_ = Urho3D::Clamp(cameraAngles_.x_, -90.0f, 90.0f);

                cameraNode_.SetRotation(Urho3D::Quaternion(cameraAngles_.x_, cameraAngles_.y_, 0));

                if (hotKeyMode == HotKeyMode::Standard)
                {
                    if (input.IsMouseButtonDown(Qt::MiddleButton) && p.hasSelection_)
                    {
                        Vector3 d = cameraNode_.GetWorldPosition() - p.selectionCenter_;
                        cameraNode_.SetWorldPosition(p.selectionCenter_ - cameraNode_.GetWorldRotation() * Vector3(0.0, 0.0, d.Length()));
                        orbiting_ = true;
                    }
                }
                else if (hotKeyMode == HotKeyMode::Blender)
                {
                    if (input.IsMouseButtonDown(Qt::MiddleButton))
                    {
                        Vector3 d = cameraNode_.GetWorldPosition() - p.selectionCenter_;
                        cameraNode_.SetWorldPosition(p.selectionCenter_ - cameraNode_.GetWorldRotation() * Vector3(0.0, 0.0, d.Length()));
                        orbiting_ = true;
                    }
                }
            }
        }
    }
    else
        input.SetMouseMode(MM_ABSOLUTE);

    if (orbiting_ && !input.IsMouseButtonDown(Qt::MiddleButton))
        orbiting_ = false;

    if (hotKeyMode == HotKeyMode::Blender)
    {
        // Implement 'View Closer' here if you want
    }
#endif
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
    Camera* newCamera = static_cast<Camera*>(eventData[EditorCurrentViewportChanged::P_CAMERA].GetPtr());
    SetCamera(newCamera);
}

}


#if 0

#include "SceneViewportManager.h"
#include "SceneDocument.h"
#include "SceneEditor.h"
#include "../Configuration.h"
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>

namespace Urho3D
{

/// Get number of viewports.
int GetNumberOfViewports(SceneViewportLayout layout)
{
    switch (layout)
    {
    case Urho3DEditor::SceneViewportLayout::Single:
        return 1;
    case Urho3DEditor::SceneViewportLayout::Vertical:
    case Urho3DEditor::SceneViewportLayout::Horizontal:
        return 2;
    case Urho3DEditor::SceneViewportLayout::Top1_Bottom2:
    case Urho3DEditor::SceneViewportLayout::Top2_Bottom1:
    case Urho3DEditor::SceneViewportLayout::Left1_Right2:
    case Urho3DEditor::SceneViewportLayout::Left2_Right1:
        return 3;
    case Urho3DEditor::SceneViewportLayout::Quad:
        return 4;
    case Urho3DEditor::SceneViewportLayout::Empty:
    default:
        return 0;
    }
}

SceneViewport::SceneViewport(Urho3D::Context* context, Urho3D::Scene* scene, Urho3D::Camera* camera)
    : sceneCamera_(camera)
    , cameraNode_(context)
    , camera_(*cameraNode_.CreateComponent<Urho3D::Camera>())
    , viewportCamera_(sceneCamera_ ? sceneCamera_ : &camera_)
    , viewport_(new Urho3D::Viewport(context, scene, viewportCamera_))
    , flyMode_(false)
    , orbiting_(false)
{
}

void SceneViewport::SetTransform(const Urho3D::Vector3& position, const Urho3D::Quaternion& rotation)
{
    cameraNode_.SetWorldPosition(position);
    cameraNode_.SetWorldRotation(rotation);
    cameraAngles_ = cameraNode_.GetRotation().EulerAngles();
}

void SceneViewport::SetRect(Urho3D::IntRect rect)
{
    viewport_->SetRect(rect);
}

void SceneViewport::Update(const SceneViewportUpdateParams& p)
{
    using namespace Urho3D;
    const float timeStep = p.timeStep_;
    SceneInputInterface& input = *p.input_;
    Configuration& config = *p.config_;

    const HotKeyMode hotKeyMode = (HotKeyMode)config.GetValue(SceneEditor::VarHotKeyMode).toInt();
    const float cameraShiftSpeedMultiplier = config.GetValue(SceneEditor::VarCameraShiftSpeedMultiplier).toFloat();
    const float cameraBaseSpeed = config.GetValue(SceneEditor::VarCameraBaseSpeed).toFloat();
    const bool mouseWheelCameraPosition = config.GetValue(SceneEditor::VarMouseWheelCameraPosition).toBool();
    const bool mmbPanMode = config.GetValue(SceneEditor::VarMmbPanMode).toBool();
    const bool limitRotation = config.GetValue(SceneEditor::VarLimitRotation).toBool();
    const float cameraBaseRotationSpeed = config.GetValue(SceneEditor::VarCameraBaseRotationSpeed).toFloat();

    // Check for camera fly mode
    if (hotKeyMode == HotKeyMode::Blender)
    {
        if (input.IsKeyDown(Qt::Key_Shift) && input.IsKeyPressed(Qt::Key_F))
            flyMode_ = !flyMode_;
    }

    // Move camera
    float speedMultiplier = 1.0;
    if (input.IsKeyDown(Qt::Key_Shift))
        speedMultiplier = cameraShiftSpeedMultiplier;

    if (!input.IsKeyDown(Qt::Key_Control) && !input.IsKeyDown(Qt::Key_Alt))
    {
        if (hotKeyMode == HotKeyMode::Standard || (hotKeyMode == HotKeyMode::Blender && flyMode_ && !input.IsKeyDown(Qt::Key_Shift)))
        {
            if (input.IsKeyDown(Qt::Key_W) || input.IsKeyDown(Qt::Key_Up))
            {
                cameraNode_.Translate(Vector3(0, 0, cameraBaseSpeed) * timeStep * speedMultiplier);
            }
            if (input.IsKeyDown(Qt::Key_S) || input.IsKeyDown(Qt::Key_Down))
            {
                cameraNode_.Translate(Vector3(0, 0, -cameraBaseSpeed) * timeStep * speedMultiplier);
            }
            if (input.IsKeyDown(Qt::Key_A) || input.IsKeyDown(Qt::Key_Left))
            {
                cameraNode_.Translate(Vector3(-cameraBaseSpeed, 0, 0) * timeStep * speedMultiplier);
            }
            if (input.IsKeyDown(Qt::Key_D) || input.IsKeyDown(Qt::Key_Right))
            {
                cameraNode_.Translate(Vector3(cameraBaseSpeed, 0, 0) * timeStep * speedMultiplier);
            }
            if (input.IsKeyDown(Qt::Key_E) || input.IsKeyDown(Qt::Key_PageUp))
            {
                cameraNode_.Translate(Vector3(0, cameraBaseSpeed, 0) * timeStep * speedMultiplier, TS_WORLD);
            }
            if (input.IsKeyDown(Qt::Key_Q) || input.IsKeyDown(Qt::Key_PageDown))
            {
                cameraNode_.Translate(Vector3(0, -cameraBaseSpeed, 0) * timeStep * speedMultiplier, TS_WORLD);
            }
        }
    }

    if (input.GetMouseWheelMove() != 0)
    {
        if (hotKeyMode == HotKeyMode::Standard)
        {
            if (mouseWheelCameraPosition)
            {
                cameraNode_.Translate(Vector3(0, 0, -cameraBaseSpeed) * -input.GetMouseWheelMove() * 20 * timeStep *
                    speedMultiplier);
            }
            else
            {
                const float zoom = camera_.GetZoom() + -input.GetMouseWheelMove() * 0.1 * speedMultiplier;
                camera_.SetZoom(Clamp(zoom, 0.1f, 30.0f));
            }
        }
        else if (hotKeyMode == HotKeyMode::Blender)
        {
            if (mouseWheelCameraPosition && !camera_.IsOrthographic())
            {
                if (input.IsKeyDown(Qt::Key_Shift))
                    cameraNode_.Translate(Vector3(0, -cameraBaseSpeed, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                else if (input.IsKeyDown(Qt::Key_Control))
                    cameraNode_.Translate(Vector3(-cameraBaseSpeed, 0, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                else
                {
                    float distance = (cameraNode_.GetWorldPosition() - p.selectionCenter_).Length();
                    float ratio = distance / 40.0f;
                    float factor = ratio < 1.0f ? ratio : 1.0f;
                    cameraNode_.Translate(Vector3(0, 0, -cameraBaseSpeed) * -input.GetMouseWheelMove() * 40 * factor*timeStep*speedMultiplier);
                }
            }
            else
            {
                if (input.IsKeyDown(Qt::Key_Shift))
                {
                    cameraNode_.Translate(Vector3(0, -cameraBaseSpeed, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                }
                else if (input.IsKeyDown(Qt::Key_Control))
                {
                    cameraNode_.Translate(Vector3(-cameraBaseSpeed, 0, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                }
                else
                {
                    if (input.IsKeyDown(Qt::Key_Alt))
                    {
                        const float zoom = camera_.GetZoom() + -input.GetMouseWheelMove() * 0.1f * speedMultiplier;
                        camera_.SetZoom(Clamp(zoom, 0.1f, 30.0f));
                    }
                    else
                    {
                        cameraNode_.Translate(Vector3(0, 0, -cameraBaseSpeed) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                    }
                }
            }
        }
    }

    if (input.IsKeyDown(Qt::Key_Home))
    {
        if (p.hasSelection_)
        {
            Vector3 d = cameraNode_.GetWorldPosition() - p.selectionCenter_;
            cameraNode_.SetWorldPosition(p.selectionCenter_ - cameraNode_.GetRotation() * Vector3(0.0, 0.0, 10));
        }
    }

    // Rotate/orbit/pan camera
    bool changeCamViewButton = false;

    if (hotKeyMode == HotKeyMode::Standard)
        changeCamViewButton = input.IsMouseButtonDown(Qt::RightButton) || input.IsMouseButtonDown(Qt::MiddleButton);
    else if (hotKeyMode == HotKeyMode::Blender)
    {
        changeCamViewButton = input.IsMouseButtonDown(Qt::MiddleButton) || flyMode_;

        if (input.IsMouseButtonDown(Qt::RightButton) || input.IsKeyDown(Qt::Key_Escape))
            flyMode_ = false;
    }

    if (changeCamViewButton)
    {
        input.SetMouseMode(MM_WRAP);

        const IntVector2 mouseMove = input.GetMouseMove();
        if (mouseMove.x_ != 0 || mouseMove.y_ != 0)
        {
            bool panTheCamera = false;

            if (hotKeyMode == HotKeyMode::Standard)
            {
                if (input.IsMouseButtonDown(Qt::MiddleButton))
                {
                    if (mmbPanMode)
                        panTheCamera = !input.IsKeyDown(Qt::Key_Shift);
                    else
                        panTheCamera = input.IsKeyDown(Qt::Key_Shift);
                }
            }
            else if (hotKeyMode == HotKeyMode::Blender)
            {
                if (!flyMode_)
                    panTheCamera = input.IsKeyDown(Qt::Key_Shift);
            }

            if (panTheCamera)
                cameraNode_.Translate(Vector3(-mouseMove.x_, mouseMove.y_, 0) * timeStep * cameraBaseSpeed * 0.5);
            else
            {
                cameraAngles_.x_ += mouseMove.y_ * cameraBaseRotationSpeed;
                cameraAngles_.y_ += mouseMove.x_ * cameraBaseRotationSpeed;

                if (limitRotation)
                    cameraAngles_.x_ = Urho3D::Clamp(cameraAngles_.x_, -90.0f, 90.0f);

                cameraNode_.SetRotation(Urho3D::Quaternion(cameraAngles_.x_, cameraAngles_.y_, 0));

                if (hotKeyMode == HotKeyMode::Standard)
                {
                    if (input.IsMouseButtonDown(Qt::MiddleButton) && p.hasSelection_)
                    {
                        Vector3 d = cameraNode_.GetWorldPosition() - p.selectionCenter_;
                        cameraNode_.SetWorldPosition(p.selectionCenter_ - cameraNode_.GetWorldRotation() * Vector3(0.0, 0.0, d.Length()));
                        orbiting_ = true;
                    }
                }
                else if (hotKeyMode == HotKeyMode::Blender)
                {
                    if (input.IsMouseButtonDown(Qt::MiddleButton))
                    {
                        Vector3 d = cameraNode_.GetWorldPosition() - p.selectionCenter_;
                        cameraNode_.SetWorldPosition(p.selectionCenter_ - cameraNode_.GetWorldRotation() * Vector3(0.0, 0.0, d.Length()));
                        orbiting_ = true;
                    }
                }
            }
        }
    }
    else
        input.SetMouseMode(MM_ABSOLUTE);

    if (orbiting_ && !input.IsMouseButtonDown(Qt::MiddleButton))
        orbiting_ = false;

    if (hotKeyMode == HotKeyMode::Blender)
    {
        // Implement 'View Closer' here if you want
    }
}

//////////////////////////////////////////////////////////////////////////
SceneViewportManager::SceneViewportManager(SceneDocument& document)
    : Object(document.GetContext())
    , document_(document)
    , scene_(document_.GetScene())
    , graphics_(*GetSubsystem<Urho3D::Graphics>())
    , currentViewport_(0)
    , layout_(SceneViewportLayout::Empty)
{
    SetLayout(SceneViewportLayout::Single); // #TODO Change
    SubscribeToEvent(Urho3D::E_SCREENMODE, URHO3D_HANDLER(SceneViewportManager, HandleResize));
}

void SceneViewportManager::SetLayout(SceneViewportLayout layout)
{
    layout_ = layout;
    UpdateNumberOfViewports(GetNumberOfViewports(layout));
    UpdateViewportLayout();
    emit viewportsChanged();
}

void SceneViewportManager::ApplyViewports()
{
    Urho3D::Renderer* renderer = GetSubsystem<Urho3D::Renderer>();
    unsigned index = 0;
    for (QSharedPointer<SceneViewport>& viewport : viewports_)
        renderer->SetViewport(index++, &viewport->GetViewport());
    while (index < renderer->GetNumViewports())
        renderer->SetViewport(index++, nullptr);

    currentViewport_ = qMin(currentViewport_, (int)index - 1);
}

Urho3D::Ray SceneViewportManager::ComputeCameraRay(const Urho3D::Viewport& viewport, const Urho3D::IntVector2& mousePosition)
{
    using namespace Urho3D;

    const IntRect rect = viewport.GetRect().Size() == IntVector2::ZERO
        ? IntRect(0, 0, graphics_.GetWidth(), graphics_.GetHeight())
        : viewport.GetRect();

    return viewport.GetCamera()->GetScreenRay(
        float(mousePosition.x_ - rect.left_) / rect.Width(),
        float(mousePosition.y_ - rect.top_) / rect.Height());
}

Urho3D::Camera& SceneViewportManager::GetCurrentCamera()
{
    return viewports_[currentViewport_]->GetCamera();
}

void SceneViewportManager::Update(SceneInputInterface& input, float timeStep)
{
    // Select current viewport
    if (!input.IsMouseButtonDown(Qt::LeftButton)
        && !input.IsMouseButtonDown(Qt::RightButton) && !input.IsMouseButtonDown(Qt::MiddleButton))
    {
        SelectCurrentViewport(input.GetMousePosition());
    }

    // Update current viewport
    SceneViewportUpdateParams param;
    param.timeStep_ = timeStep;
    param.config_ = &document_.GetConfig();
    param.input_ = &input;
    param.hasSelection_ = document_.HasSelectedNodesOrComponents();
    param.selectionCenter_ = document_.GetSelectedCenter();

    SceneViewport& currentViewport = *viewports_[currentViewport_];
    currentCameraRay_ = ComputeCameraRay(currentViewport.GetViewport(), input.GetMousePosition());
    currentViewport.Update(param);
}

void SceneViewportManager::HandleResize(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    UpdateViewportLayout();
}

void SceneViewportManager::UpdateNumberOfViewports(int numViewports)
{
    using namespace Urho3D;

    static const Vector3 defaultPosition(0, 10, -10);
    static const Quaternion defaultRotation(45, 0, 0);

    const int oldNumViewports = viewports_.size();
    if (numViewports < oldNumViewports)
        viewports_.resize(numViewports);
    else
    {
        Vector3 position = defaultPosition;
        Quaternion rotation = defaultRotation;
        if (oldNumViewports > 0)
        {
            Node& node = viewports_[oldNumViewports - 1]->GetNode();
            position = node.GetWorldPosition();
            rotation = node.GetWorldRotation();
        }

        for (int i = oldNumViewports; i < numViewports; ++i)
        {
            QSharedPointer<SceneViewport> viewport(new SceneViewport(context_, &scene_, nullptr));
            viewport->SetTransform(position, rotation);
            viewports_.push_back(viewport);
        }
    }
}

void SceneViewportManager::SelectCurrentViewport(const Urho3D::IntVector2& mousePosition)
{
    Urho3D::IntVector2 localPosition;
    for (int i = 0; i < viewports_.size(); ++i)
    {
        const Urho3D::Viewport& viewport = viewports_[i]->GetViewport();
        const Urho3D::IntRect rect = viewport.GetRect();
        if (rect.Size() == Urho3D::IntVector2::ZERO || rect.IsInside(mousePosition) != Urho3D::OUTSIDE)
        {
            currentViewport_ = i;
            break;
        }
    }
}

void SceneViewportManager::UpdateViewportLayout()
{
    using namespace Urho3D;
    Graphics* graphics = GetSubsystem<Graphics>();
    const int width = graphics->GetWidth();
    const int height = graphics->GetHeight();
    const int halfWidth = width / 2;
    const int halfHeight = height / 2;

    Q_ASSERT(viewports_.size() == GetNumberOfViewports(layout_));
    switch (layout_)
    {
    case Urho3DEditor::SceneViewportLayout::Empty:
        break;
    case Urho3DEditor::SceneViewportLayout::Single:
        viewports_[0]->SetRect(IntRect(0, 0, width, height));
        break;
    case Urho3DEditor::SceneViewportLayout::Vertical:
        viewports_[0]->SetRect(IntRect(0, 0, halfWidth, height));
        viewports_[1]->SetRect(IntRect(halfWidth, 0, width, height));
        break;
    case Urho3DEditor::SceneViewportLayout::Horizontal:
        viewports_[0]->SetRect(IntRect(0, 0, width, halfHeight));
        viewports_[1]->SetRect(IntRect(0, halfHeight, width, height));
        break;
    case Urho3DEditor::SceneViewportLayout::Quad:
        viewports_[0]->SetRect(IntRect(0, 0, halfWidth, halfHeight));
        viewports_[1]->SetRect(IntRect(halfWidth, 0, width, halfHeight));
        viewports_[2]->SetRect(IntRect(0, halfHeight, halfWidth, height));
        viewports_[3]->SetRect(IntRect(halfWidth, halfHeight, width, height));
        break;
    case Urho3DEditor::SceneViewportLayout::Top1_Bottom2:
        viewports_[0]->SetRect(IntRect(0, 0, width, halfHeight));
        viewports_[1]->SetRect(IntRect(0, halfHeight, halfWidth, height));
        viewports_[2]->SetRect(IntRect(halfWidth, halfHeight, width, height));
        break;
    case Urho3DEditor::SceneViewportLayout::Top2_Bottom1:
        viewports_[0]->SetRect(IntRect(0, 0, halfWidth, halfHeight));
        viewports_[1]->SetRect(IntRect(halfWidth, 0, width, halfHeight));
        viewports_[2]->SetRect(IntRect(0, halfHeight, width, height));
        break;
    case Urho3DEditor::SceneViewportLayout::Left1_Right2:
        viewports_[0]->SetRect(IntRect(0, 0, halfWidth, height));
        viewports_[1]->SetRect(IntRect(halfWidth, 0, width, halfHeight));
        viewports_[2]->SetRect(IntRect(halfWidth, halfHeight, width, height));
        break;
    case Urho3DEditor::SceneViewportLayout::Left2_Right1:
        viewports_[0]->SetRect(IntRect(0, 0, halfWidth, halfHeight));
        viewports_[1]->SetRect(IntRect(0, halfHeight, halfWidth, height));
        viewports_[2]->SetRect(IntRect(halfWidth, 0, width, height));
        break;
    default:
        break;
    }
}

}

#endif

