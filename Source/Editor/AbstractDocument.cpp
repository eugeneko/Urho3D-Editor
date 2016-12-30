#include "AbstractDocument.h"
#include "EditorSettings.h"
#include "Urho3DWidget.h"
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/IOEvents.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Resource/XMLFile.h>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

namespace Urho3D
{

AbstractPage::AbstractPage(Context* context)
    : Object(context)
    , unsavedChanges_(false)
{
}

bool AbstractPage::LaunchSaveDialog()
{
    EditorSettings* settings = GetSubsystem<EditorSettings>();

    QFileDialog dialog;
    dialog.selectFile(GetDefaultFileName());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(settings->GetLastDirectory());
    dialog.setNameFilter(GetFilters());
    if (!dialog.exec())
        return false;

    const QStringList files = dialog.selectedFiles();
    if (files.isEmpty())
        return false;

    fileName_ = files[0];
    settings->SetLastDirectory(fileName_);
    SetTitle(QFileInfo(fileName_).fileName());
    return true;
}

bool AbstractPage::Save(bool askForDestination)
{
    if (fileName_.isEmpty() || askForDestination)
    {
        if (!LaunchSaveDialog())
            return false;
    }
    if (DoSave())
    {
        ResetUnsaved();
        return true;
    }
    return false;
}

bool AbstractPage::LaunchOpenDialog()
{
    EditorSettings* settings = GetSubsystem<EditorSettings>();

    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(settings->GetLastDirectory());
    dialog.setNameFilter(GetFilters());
    if (!dialog.exec())
        return false;

    const QStringList files = dialog.selectedFiles();
    if (files.isEmpty())
        return false;

    fileName_ = files[0];
    settings->SetLastDirectory(fileName_);
    SetTitle(QFileInfo(fileName_).fileName());
    return true;
}

bool AbstractPage::Open(bool askForSource)
{
    if (fileName_.isEmpty() || askForSource)
    {
        if (!LaunchOpenDialog())
            return false;
    }
    ResetUnsaved();
    return DoLoad();
}

void AbstractPage::MarkUnsaved()
{
    const bool wasUnsaved = unsavedChanges_;
    unsavedChanges_ = true;
    if (!wasUnsaved)
        emit titleChanged(this, GetTitleDecorated());
}

void AbstractPage::ResetUnsaved()
{
    const bool wasUnsaved = unsavedChanges_;
    unsavedChanges_ = false;
    if (wasUnsaved)
        emit titleChanged(this, GetTitleDecorated());
}

void AbstractPage::SetTitle(const QString& value)
{
    title_ = value;
    emit titleChanged(this, GetTitleDecorated());
}

//////////////////////////////////////////////////////////////////////////
StartPage::StartPage(Context* context)
    : AbstractPage(context)
    , layout_(new QGridLayout(this))
    , newProjectButton_(new QPushButton("New Project"))
    , recentProjects_(new QListWidget())
{
    setLayout(layout_);
    layout_->addWidget(newProjectButton_, 0, 0);
    layout_->addWidget(new QLabel("Recent projects:"), 1, 0);
    layout_->addWidget(recentProjects_, 2, 0);
    SetTitle("Start Page");
}

//////////////////////////////////////////////////////////////////////////
Urho3DPage::Urho3DPage(Urho3DWidget* urho3dWidget, const QString& name)
    : AbstractPage(urho3dWidget->GetContext())
    , urho3dWidget_(urho3dWidget)
{
    SetTitle(name);
}

Urho3DPage::~Urho3DPage()
{
}

Context* Urho3DPage::GetContext() const
{
    return urho3dWidget_->GetContext();
}

//////////////////////////////////////////////////////////////////////////
SceneEditorPage::SceneEditorPage(Urho3DWidget* urho3dWidget, const QString& name)
    : Urho3DPage(urho3dWidget, name)
    , cameraNode_(urho3dWidget->GetContext())
    , camera_(cameraNode_.CreateComponent<Camera>())
{
    cameraNode_.SetWorldPosition(Vector3(0, 1, -1));
    cameraNode_.LookAt(Vector3::ZERO);

//     grabKeyboard();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneEditorPage, HandleUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(SceneEditorPage, HandleKey));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(SceneEditorPage, HandleKey));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(SceneEditorPage, HandleMouseButton));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(SceneEditorPage, HandleMouseButton));
}

void SceneEditorPage::SetScene(SharedPtr<Scene> scene)
{
    scene_ = scene;
    viewport_ = new Viewport(GetContext(), scene_, camera_);
    SetupViewport();
}

void SceneEditorPage::SetupViewport()
{
//     if (IsActive())
    {
        Renderer* renderer = GetContext()->GetSubsystem<Renderer>();
        renderer->SetViewport(0, viewport_);
    }
}

void SceneEditorPage::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
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

void SceneEditorPage::HandleKey(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    if (eventType == E_KEYDOWN)
    {
        const int button = eventData[KeyDown::P_KEY].GetInt();
        if (button == 'W')
        {
            cameraNode_.Translate(Vector3::FORWARD);
        }
    }
    else if (eventType == E_KEYUP)
    {
        const int button = eventData[KeyDown::P_KEY].GetInt();
    }
}

void SceneEditorPage::HandleMouseButton(StringHash eventType, VariantMap& eventData)
{
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
