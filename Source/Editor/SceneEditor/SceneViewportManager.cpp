#include "SceneViewportManager.h"
#include "SceneDocument.h"
#include "SceneEditor.h"
#include "../Configuration.h"
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>

namespace Urho3DEditor
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
    , localCameraNode_(context)
    , localCamera_(localCameraNode_.CreateComponent<Urho3D::Camera>())
    , camera_(sceneCamera_ ? sceneCamera_ : localCamera_)
    , viewport_(new Urho3D::Viewport(context, scene, camera_))
{
    localCameraNode_.SetWorldPosition(Urho3D::Vector3(0, 10, -10));
    localCameraNode_.LookAt(Urho3D::Vector3::ZERO);
    localCameraAngles_ = localCameraNode_.GetRotation().EulerAngles();
}

void SceneViewport::SetRect(Urho3D::IntRect rect)
{
    viewport_->SetRect(rect);
}

void SceneViewport::UpdateRotation(float deltaPitch, float deltaYaw, bool limitPitch)
{
    localCameraAngles_.x_ += deltaPitch;
    localCameraAngles_.y_ += deltaYaw;

    if (limitPitch)
        localCameraAngles_.x_ = Urho3D::Clamp(localCameraAngles_.x_, -90.0f, 90.0f);

    localCameraNode_.SetRotation(Urho3D::Quaternion(localCameraAngles_.x_, localCameraAngles_.y_, 0));
}

//////////////////////////////////////////////////////////////////////////
SceneViewportManager::SceneViewportManager(SceneDocument& document)
    : Object(document.GetContext())
    , document_(document)
    , scene_(document_.GetScene())
    , graphics_(*GetSubsystem<Urho3D::Graphics>())
    , currentViewport_(0)
    , layout_(SceneViewportLayout::Empty)
    , flyMode_(false)
    , orbiting_(false)
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

void SceneViewportManager::UpdateCurrentViewport(const Urho3D::IntVector2& mousePosition)
{
    Urho3D::IntVector2 localPosition;
    for (int i = 0; i < viewports_.size(); ++i)
    {
        const Urho3D::Viewport& viewport = viewports_[i]->GetViewport();
        const Urho3D::IntRect rect = viewport.GetRect();
        if (rect.Size() == Urho3D::IntVector2::ZERO || rect.IsInside(mousePosition) != Urho3D::OUTSIDE)
        {
            currentViewport_ = i;
            currentCameraRay_ = ComputeCameraRay(viewport, mousePosition);
            break;
        }
    }
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

void SceneViewportManager::Update(SceneInputInterface& input, const Urho3D::Ray& cameraRay, float timeStep)
{
    Configuration& config = document_.GetConfig();
    const HotKeyMode hotKeyMode = (HotKeyMode)config.GetValue(SceneEditor::VarHotkeyMode).toInt();
    const float cameraShiftSpeedMultiplier = 5.0f; // #TODO Make config
    const float cameraBaseSpeed = 5.0f;
    const bool mouseWheelCameraPosition = false;
    const bool mmbPanMode = true;
    const bool limitRotation = true;
    const float cameraBaseRotationSpeed = 0.2f;

    using namespace Urho3D;
    SceneViewport& currentViewport = *viewports_[currentViewport_];
    Node& cameraNode = currentViewport.GetNode();
    Camera& camera = currentViewport.GetCamera();

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
                cameraNode.Translate(Vector3(0, 0, cameraBaseSpeed) * timeStep * speedMultiplier);
            }
            if (input.IsKeyDown(Qt::Key_S) || input.IsKeyDown(Qt::Key_Down))
            {
                cameraNode.Translate(Vector3(0, 0, -cameraBaseSpeed) * timeStep * speedMultiplier);
            }
            if (input.IsKeyDown(Qt::Key_A) || input.IsKeyDown(Qt::Key_Left))
            {
                cameraNode.Translate(Vector3(-cameraBaseSpeed, 0, 0) * timeStep * speedMultiplier);
            }
            if (input.IsKeyDown(Qt::Key_D) || input.IsKeyDown(Qt::Key_Right))
            {
                cameraNode.Translate(Vector3(cameraBaseSpeed, 0, 0) * timeStep * speedMultiplier);
            }
            if (input.IsKeyDown(Qt::Key_E) || input.IsKeyDown(Qt::Key_PageUp))
            {
                cameraNode.Translate(Vector3(0, cameraBaseSpeed, 0) * timeStep * speedMultiplier, TS_WORLD);
            }
            if (input.IsKeyDown(Qt::Key_Q) || input.IsKeyDown(Qt::Key_PageDown))
            {
                cameraNode.Translate(Vector3(0, -cameraBaseSpeed, 0) * timeStep * speedMultiplier, TS_WORLD);
            }
        }
    }

    if (input.GetMouseWheelMove() != 0)
    {
        if (hotKeyMode == HotKeyMode::Standard)
        {
            if (mouseWheelCameraPosition)
            {
                cameraNode.Translate(Vector3(0, 0, -cameraBaseSpeed) * -input.GetMouseWheelMove() * 20 * timeStep *
                    speedMultiplier);
            }
            else
            {
                const float zoom = camera.GetZoom() + -input.GetMouseWheelMove() * 0.1 * speedMultiplier;
                camera.SetZoom(Clamp(zoom, 0.1f, 30.0f));
            }
        }
        else if (hotKeyMode == HotKeyMode::Blender)
        {
            if (mouseWheelCameraPosition && !camera.IsOrthographic())
            {
                if (input.IsKeyDown(Qt::Key_Shift))
                    cameraNode.Translate(Vector3(0, -cameraBaseSpeed, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                else if (input.IsKeyDown(Qt::Key_Control))
                    cameraNode.Translate(Vector3(-cameraBaseSpeed, 0, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                else
                {
                    Vector3 center = document_.GetSelectedCenter();
                    float distance = (cameraNode.GetWorldPosition() - center).Length();
                    float ratio = distance / 40.0f;
                    float factor = ratio < 1.0f ? ratio : 1.0f;
                    cameraNode.Translate(Vector3(0, 0, -cameraBaseSpeed) * -input.GetMouseWheelMove() * 40 * factor*timeStep*speedMultiplier);
                }
            }
            else
            {
                if (input.IsKeyDown(Qt::Key_Shift))
                {
                    cameraNode.Translate(Vector3(0, -cameraBaseSpeed, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                }
                else if (input.IsKeyDown(Qt::Key_Control))
                {
                    cameraNode.Translate(Vector3(-cameraBaseSpeed, 0, 0) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                }
                else
                {
                    if (input.IsKeyDown(Qt::Key_Alt))
                    {
                        const float zoom = camera.GetZoom() + -input.GetMouseWheelMove() * 0.1f * speedMultiplier;
                        camera.SetZoom(Clamp(zoom, 0.1f, 30.0f));
                    }
                    else
                    {
                        cameraNode.Translate(Vector3(0, 0, -cameraBaseSpeed) * -input.GetMouseWheelMove() * 20 * timeStep * speedMultiplier);
                    }
                }
            }
        }
    }

    if (input.IsKeyDown(Qt::Key_Home))
    {
        if (document_.HasSelectedNodesOrComponents())
        {
            Vector3 centerPoint = document_.GetSelectedCenter();
            Vector3 d = cameraNode.GetWorldPosition() - centerPoint;
            cameraNode.SetWorldPosition(centerPoint - cameraNode.GetRotation() * Vector3(0.0, 0.0, 10));
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
                cameraNode.Translate(Vector3(-mouseMove.x_, mouseMove.y_, 0) * timeStep * cameraBaseSpeed * 0.5);
            else
            {
                currentViewport.UpdateRotation(
                    mouseMove.y_ * cameraBaseRotationSpeed, mouseMove.x_ * cameraBaseRotationSpeed, limitRotation);

                if (hotKeyMode == HotKeyMode::Standard)
                {
                    if (input.IsMouseButtonDown(Qt::MiddleButton) && document_.HasSelectedNodesOrComponents())
                    {
                        Vector3 centerPoint = document_.GetSelectedCenter();
                        Vector3 d = cameraNode.GetWorldPosition() - centerPoint;
                        cameraNode.SetWorldPosition(centerPoint - cameraNode.GetWorldRotation() * Vector3(0.0, 0.0, d.Length()));
                        orbiting_ = true;
                    }
                }
                else if (hotKeyMode == HotKeyMode::Blender)
                {
                    if (input.IsMouseButtonDown(Qt::MiddleButton))
                    {
                        const Vector3 centerPoint = document_.GetSelectedCenter();

                        Vector3 d = cameraNode.GetWorldPosition() - centerPoint;
                        cameraNode.SetWorldPosition(centerPoint - cameraNode.GetWorldRotation() * Vector3(0.0, 0.0, d.Length()));
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

void SceneViewportManager::HandleResize(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    UpdateViewportLayout();
}

void SceneViewportManager::UpdateNumberOfViewports(int numViewports)
{
    viewports_.push_back(QSharedPointer<SceneViewport>(new SceneViewport(context_, &scene_, nullptr)));
}

void SceneViewportManager::UpdateViewportLayout()
{
    using namespace Urho3D;
    Graphics* graphics = GetSubsystem<Graphics>();
    const int width = graphics->GetWidth();
    const int height = graphics->GetHeight();

    switch (layout_)
    {
    case Urho3DEditor::SceneViewportLayout::Empty:
        Q_ASSERT(viewports_.size() == 0);
        break;
    case Urho3DEditor::SceneViewportLayout::Single:
        Q_ASSERT(viewports_.size() == 1);
        viewports_[0]->SetRect(IntRect(0, 0, width - 1, height - 1));
        break;
    case Urho3DEditor::SceneViewportLayout::Vertical:
        break;
    case Urho3DEditor::SceneViewportLayout::Horizontal:
        break;
    case Urho3DEditor::SceneViewportLayout::Quad:
        break;
    case Urho3DEditor::SceneViewportLayout::Top1_Bottom2:
        break;
    case Urho3DEditor::SceneViewportLayout::Top2_Bottom1:
        break;
    case Urho3DEditor::SceneViewportLayout::Left1_Right2:
        break;
    case Urho3DEditor::SceneViewportLayout::Left2_Right1:
        break;
    default:
        break;
    }
}

}
