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

AbstractDocument::AbstractDocument(Context* context)
    : Object(context)
    , isDirty_(false)
{

}

void AbstractDocument::MarkDirty()
{
    isDirty_ = true;
}

bool AbstractDocument::Close()
{
    return true;
}

void AbstractDocument::SaveAs()
{
    EditorSettings* settings = GetSubsystem<EditorSettings>();

    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(settings->GetLastDirectory());
    if (dialog.exec())
    {
        const QStringList files = dialog.selectedFiles();
        const QString fileName = files.empty() ? "" : files[0];
        if (!fileName.isEmpty())
        {
            settings->SetLastDirectory(fileName);
            SaveAs(fileName);
        }
    }
}

void AbstractDocument::SaveAs(const QString& fileName)
{
    UpdateFileNameAndTitle(fileName);
    DoSave(fileName);
}

void AbstractDocument::Open()
{
    EditorSettings* settings = GetSubsystem<EditorSettings>();

    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(settings->GetLastDirectory());
    if (dialog.exec())
    {
        const QStringList files = dialog.selectedFiles();
        const QString fileName = files.empty() ? "" : files[0];
        if (!fileName.isEmpty())
        {
            settings->SetLastDirectory(fileName);
            Open(fileName);
        }
    }
}

void AbstractDocument::Open(const QString& fileName)
{
    UpdateFileNameAndTitle(fileName);
    DoLoad(fileName);
}

void AbstractDocument::Activate()
{
    if (!isActive_)
    {
        isActive_ = true;
        DoActivate();
    }
}

void AbstractDocument::Deactivate()
{
    if (isActive_)
    {
        isActive_ = false;
        DoDeactivate();
    }
}

void AbstractDocument::UpdateFileNameAndTitle(const QString& fileName)
{
    QFileInfo fileInfo(fileName);
    fileName_ = fileInfo.absoluteFilePath();
    title_ = fileInfo.fileName();
}

//////////////////////////////////////////////////////////////////////////
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
Urho3DDocument::Urho3DDocument(Urho3DWidget* urho3dWidget, const QString& name)
    : AbstractDocument(urho3dWidget->GetContext())
    , urho3dWidget_(urho3dWidget)
{
    SetTitle(name);
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
//     if (!layout())
//         setLayout(new QVBoxLayout(this));
//     layout()->addWidget(urho3dWidget_);
}

void Urho3DDocument::DoDeactivate()
{
//     if (urho3dWidget_->parent() == this)
//         urho3dWidget_->setParent(nullptr);
}

//////////////////////////////////////////////////////////////////////////
SceneDocument::SceneDocument(Urho3DWidget* urho3dWidget, const QString& name)
    : Urho3DDocument(urho3dWidget, name)
    , cameraNode_(urho3dWidget->GetContext())
    , camera_(cameraNode_.CreateComponent<Camera>())
{
    cameraNode_.SetWorldPosition(Vector3(0, 1, -1));
    cameraNode_.LookAt(Vector3::ZERO);

//     grabKeyboard();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneDocument, HandleUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(SceneDocument, HandleKey));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(SceneDocument, HandleKey));
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

void SceneDocument::HandleKey(StringHash eventType, VariantMap& eventData)
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

ProjectDocument::ProjectDocument(Context* context)
    : AbstractDocument(context)
    , layout_(new QFormLayout())
    , fieldResourcePrefixPaths_(new QLineEdit())
    , fieldResourcePaths_(new QListWidget())
{
    layout_->addRow("Resource Prefix Paths:", fieldResourcePrefixPaths_);
    connect(fieldResourcePrefixPaths_, SIGNAL(textChanged(const QString&)), this, SLOT(OnResourcePrefixPathsChanged()));

    layout_->addRow("Resource Paths:", fieldResourcePaths_);

//     setLayout(layout_);
}

void ProjectDocument::DoSave(const QString& fileName)
{
    XMLFile xml(context_);
    XMLElement root = xml.CreateRoot("project");
    XMLElement resourcePaths = root.CreateChild("resourcepaths");
    resourcePaths.SetAttribute("prefix", fieldResourcePrefixPaths_->text().toStdString().c_str());

    File file(context_);
    if (file.Open(fileName.toStdString().c_str(), FILE_WRITE))
        xml.Save(file);
}

void ProjectDocument::DoLoad(const QString& fileName)
{
    XMLFile xml(context_);
    xml.LoadFile(fileName.toStdString().c_str());

    XMLElement root = xml.GetRoot("project");
    XMLElement resourcePaths = root.GetChild("resourcepaths");
    resourcePrefixPaths_ = resourcePaths.GetAttribute("prefix").CString();

    fieldResourcePrefixPaths_->setText(resourcePrefixPaths_);
}

void ProjectDocument::UpdateResourcePaths()
{

}

void ProjectDocument::OnResourcePrefixPathsChanged(const QString& value)
{
    resourcePrefixPaths_ = value;
}

}
