#include "EditorDocument.h"
#include "Urho3DWidget.h"
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/IOEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <QVBoxLayout>

namespace Urho3D
{

EditorDocument::EditorDocument(const QString& name)
    : QWidget()
    , name_(name)
    , isActive_(false)
{

}

void EditorDocument::Activate()
{
    if (!isActive_)
    {
        isActive_ = true;
        DoActivate();
    }
}

void EditorDocument::Deactivate()
{
    if (isActive_)
    {
        isActive_ = false;
        DoDeactivate();
    }
}

//////////////////////////////////////////////////////////////////////////
Urho3DDocument::Urho3DDocument(Urho3DWidget* urho3dWidget, const QString& name)
    : EditorDocument(name)
    , Object(urho3dWidget->GetContext())
    , urho3dWidget_(urho3dWidget)
{
}

Urho3DDocument::~Urho3DDocument()
{
}

Context* Urho3DDocument::GetContext() const
{
    return urho3dWidget_->GetContext();
}

void Urho3DDocument::DoActivate()
{
    if (!layout())
        setLayout(new QVBoxLayout(this));
    layout()->addWidget(urho3dWidget_);
}

void Urho3DDocument::DoDeactivate()
{
    if (urho3dWidget_->parent() == this)
        urho3dWidget_->setParent(nullptr);
}

//////////////////////////////////////////////////////////////////////////
SceneDocument::SceneDocument(Urho3DWidget* urho3dWidget, const QString& name)
    : Urho3DDocument(urho3dWidget, name)
    , cameraNode_(urho3dWidget->GetContext())
    , camera_(cameraNode_.CreateComponent<Camera>())
{
    cameraNode_.SetWorldPosition(Vector3(0, 1, -1));
    cameraNode_.LookAt(Vector3::ZERO);

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneDocument, HandleUpdate));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(SceneDocument, HandleMouseButton));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(SceneDocument, HandleMouseButton));
}

void SceneDocument::SetScene(SharedPtr<Scene> scene)
{
    scene_ = scene;
    viewport_ = new Viewport(GetContext(), scene_, camera_);
    SetupViewport();
}

void SceneDocument::DoActivate()
{
    Urho3DDocument::DoActivate();
    SetupViewport();
}

void SceneDocument::SetupViewport()
{
    if (IsActive())
    {
        Renderer* renderer = GetContext()->GetSubsystem<Renderer>();
        renderer->SetViewport(0, viewport_);
    }
}

void SceneDocument::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!IsActive())
        return;

    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();
    Input* input = GetSubsystem<Input>();

    if (input->GetKeyDown('W'))
        cameraNode_.Translate(Vector3::FORWARD * timeStep, TS_LOCAL);
    if (input->GetKeyDown('S'))
        cameraNode_.Translate(Vector3::BACK * timeStep, TS_LOCAL);
    if (input->GetKeyDown('A'))
        cameraNode_.Translate(Vector3::LEFT * timeStep, TS_LOCAL);
    if (input->GetKeyDown('D'))
        cameraNode_.Translate(Vector3::RIGHT * timeStep, TS_LOCAL);
    if (input->GetKeyDown('Q'))
        cameraNode_.Translate(Vector3::DOWN * timeStep, TS_WORLD);
    if (input->GetKeyDown('E'))
        cameraNode_.Translate(Vector3::UP * timeStep, TS_WORLD);

    if (input->IsMouseGrabbed())
    {
        const IntVector2 mouseMove = input->GetMouseMove();
        const Vector3 direction = cameraNode_.GetWorldDirection();
        const Quaternion rotation = cameraNode_.GetWorldRotation();
        const float yawAngle = rotation.YawAngle() + mouseMove.x_;
        const float pitchAngle = Clamp(rotation.PitchAngle() + mouseMove.y_, -89.0f, 89.0f);
        cameraNode_.SetWorldRotation(Quaternion(0.0f, yawAngle, 0.0f) * Quaternion(pitchAngle, 0.0f, 0.0f));
    }
}

void SceneDocument::HandleMouseButton(StringHash eventType, VariantMap& eventData)
{
    if (!IsActive())
        return;

    // Handle camera rotation
    Input* input = GetSubsystem<Input>();
    if (eventType == E_MOUSEBUTTONDOWN)
    {
        const int button = eventData[MouseButtonDown::P_BUTTON].GetInt();
        if (button == MOUSEB_RIGHT)
        {
            input->SetMouseVisible(false);
            input->SetMouseMode(MM_WRAP);
        }
    }
    else if (eventType == E_MOUSEBUTTONUP)
    {
        const int button = eventData[MouseButtonDown::P_BUTTON].GetInt();
        if (button == MOUSEB_RIGHT)
        {
            input->SetMouseVisible(true);
            input->SetMouseMode(MM_ABSOLUTE);
        }
    }
}

}
