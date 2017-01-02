#include "SceneEditor.h"
#include "Configuration.h"
#include "MainWindow.h"
#include "Bridge.h"
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Graphics/Renderer.h>
#include <QFileInfo>

namespace Urho3DEditor
{


SceneEditor::SceneEditor()
    : mainWindow_(nullptr)
{

}

bool SceneEditor::DoInitialize()
{
    mainWindow_ = GetModule<MainWindow>();
    if (!mainWindow_)
        return false;

    QMenu* menuFile = mainWindow_->GetTopLevelMenu(MainWindow::MenuFile);
    QAction* menuFileNew_After = mainWindow_->GetMenuAction(MainWindow::MenuFileNew_After);
    QAction* menuFileOpen_After = mainWindow_->GetMenuAction(MainWindow::MenuFileOpen_After);
    if (!menuFile || !menuFileNew_After || !menuFileOpen_After)
        return false;

    actionFileNewScene_.reset(new QAction("New Scene"));
    actionFileNewScene_->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_N);
    menuFile->insertAction(menuFileNew_After, actionFileNewScene_.data());
    connect(actionFileNewScene_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleFileNewScene()));

    actionFileOpenScene_.reset(new QAction("Open Scene..."));
    actionFileOpenScene_->setShortcut(Qt::CTRL + Qt::Key_O);
    menuFile->insertAction(menuFileOpen_After, actionFileOpenScene_.data());
    connect(actionFileOpenScene_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleFileOpenScene()));

    return true;
}

void SceneEditor::HandleFileNewScene()
{
    mainWindow_->AddPage(new ScenePage(*mainWindow_));
}

void SceneEditor::HandleFileOpenScene()
{
    QScopedPointer<ScenePage> scenePage(new ScenePage(*mainWindow_));
    if (scenePage->Open())
        mainWindow_->AddPage(scenePage.take());
}

//////////////////////////////////////////////////////////////////////////
SceneCamera::SceneCamera(Urho3D::Context* context)
    : input_(context->GetSubsystem<Urho3D::Input>())
    , cameraNode_(context)
    , camera_(cameraNode_.CreateComponent<Urho3D::Camera>())
{
    cameraNode_.SetWorldPosition(Urho3D::Vector3(0, 10, -10));
    cameraNode_.LookAt(Urho3D::Vector3::ZERO);
    angles_ = cameraNode_.GetWorldRotation().EulerAngles();
}

void SceneCamera::SetGrabMouse(bool grab)
{
    using namespace Urho3D;

    if (grab)
    {
        input_->SetMouseVisible(false);
        input_->SetMouseMode(MM_WRAP);
    }
    else
    {
        input_->SetMouseVisible(true);
        input_->SetMouseMode(MM_ABSOLUTE);
    }
}

void SceneCamera::Move(const Urho3D::Vector3& movement, const Urho3D::Vector3& rotation)
{
    using namespace Urho3D;

    cameraNode_.Translate(movement, TS_LOCAL);

    angles_ += rotation;
    angles_.y_ = Fract(angles_.y_ / 360.0f) * 360.0f;
    angles_.x_ = Clamp(angles_.x_, -85.0f, 85.0f);
    cameraNode_.SetWorldRotation(Quaternion(angles_.x_, angles_.y_, angles_.z_));
}

//////////////////////////////////////////////////////////////////////////
ScenePage::ScenePage(MainWindow& mainWindow)
    : MainWindowPage(mainWindow)
    , Object(mainWindow.GetContext())
    , widget_(mainWindow.GetUrho3DWidget())
    , camera_(context_)
    , scene_(new Urho3D::Scene(context_))
    , viewport_(new Urho3D::Viewport(context_, scene_, &camera_.GetCamera()))
{
    SetTitle("New Scene");

    SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(ScenePage, HandleUpdate));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(ScenePage, HandleMouseButton));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONUP, URHO3D_HANDLER(ScenePage, HandleMouseButton));
}

QString ScenePage::GetNameFilters()
{
    return "Urho3D Scene (*.xml *.json *.bin);;All files (*.*)";
}

void ScenePage::HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    if (!IsActive())
        return;

    using namespace Urho3D;

    const float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

    Vector3 movement;
    if (widget_->IsKeyPressed(Qt::Key_W))
        movement += Vector3::FORWARD;
    if (widget_->IsKeyPressed(Qt::Key_S))
        movement += Vector3::BACK;
    if (widget_->IsKeyPressed(Qt::Key_A))
        movement += Vector3::LEFT;
    if (widget_->IsKeyPressed(Qt::Key_D))
        movement += Vector3::RIGHT;
    if (widget_->IsKeyPressed(Qt::Key_Q))
        movement += Vector3::DOWN;
    if (widget_->IsKeyPressed(Qt::Key_E))
        movement += Vector3::UP;
    if (widget_->IsKeyPressed(Qt::Key_Shift))
        movement *= 25.0f; // #TODO Configure
    else
        movement *= 5.0f;

    Urho3D::Vector3 rotation;
    Input* input = GetSubsystem<Input>();
    if (input->IsMouseGrabbed())
    {
        const IntVector2 mouseMove = input->GetMouseMove();
        const Vector3 delta(mouseMove.y_, mouseMove.x_, 0.0f);
        const Vector3 sensitivity(0.5f, 0.5f, 0.0f); // #TODO Configure
        rotation = delta * sensitivity;
    }

    camera_.Move(movement * timeStep, rotation);
}

void ScenePage::HandleMouseButton(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;

    // Handle camera rotation
    Input* input = GetSubsystem<Input>();
    const int button = eventData[MouseButtonDown::P_BUTTON].GetInt();
    if (button == MOUSEB_RIGHT)
    {
        if (eventType == E_MOUSEBUTTONDOWN)
        {
            input->SetMouseVisible(false);
            input->SetMouseMode(MM_WRAP);
        }
        else if (eventType == E_MOUSEBUTTONUP)
        {
            input->SetMouseVisible(true);
            input->SetMouseMode(MM_ABSOLUTE);
        }
    }
}

void ScenePage::HandleCurrentPageChanged(MainWindowPage* page)
{
    if (IsActive())
    {
        Urho3D::Renderer* renderer = GetContext()->GetSubsystem<Urho3D::Renderer>();
        renderer->SetViewport(0, viewport_);
    }
}

bool ScenePage::DoLoad(const QString& fileName)
{
    Urho3D::File file(context_);
    if (!file.Open(Cast(fileName)))
        return false;

    QFileInfo fileInfo(fileName);
    if (!fileInfo.suffix().compare("xml", Qt::CaseInsensitive))
    {
        if (!scene_->LoadXML(file))
            return false;
    }
    else if (!fileInfo.suffix().compare("json", Qt::CaseInsensitive))
    {
        if (!scene_->LoadJSON(file))
            return false;
    }
    else
    {
        if (!scene_->Load(file))
            return false;
    }

    return true;
}

}
