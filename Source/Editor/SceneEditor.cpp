#include "SceneEditor.h"
#include "Configuration.h"
#include "MainWindow.h"
#include <Urho3D/IO/File.h>
#include <Urho3D/Graphics/Renderer.h>

namespace Urho3DEditor
{


SceneEditor::SceneEditor()
    : config_(nullptr)
    , mainWindow_(nullptr)
{

}

bool SceneEditor::DoInitialize()
{
    config_ = GetModule<Configuration>();
    mainWindow_ = GetModule<MainWindow>();
    if (!mainWindow_ || !config_)
        return false;

    QMenu* menuFile = mainWindow_->GetTopLevelMenu(MainWindow::MenuFile);
    QAction* menuFileNew_After = mainWindow_->GetMenuAction(MainWindow::MenuFileNew_After);
    QAction* menuFileOpen_After = mainWindow_->GetMenuAction(MainWindow::MenuFileOpen_After);
    if (!menuFile || !menuFileNew_After || !menuFileOpen_After)
        return false;

    actionFileNewScene_.reset(new QAction("New Scene"));
    menuFile->insertAction(menuFileNew_After, actionFileNewScene_.data());
    connect(actionFileNewScene_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleFileNewScene()));

    actionFileOpenScene_.reset(new QAction("Open Scene..."));
    menuFile->insertAction(menuFileOpen_After, actionFileOpenScene_.data());
    connect(actionFileOpenScene_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleFileOpenScene()));

    return true;
}

void SceneEditor::HandleFileNewScene()
{
    mainWindow_->AddPage(new ScenePage(*config_, GetContext()));
}

void SceneEditor::HandleFileOpenScene()
{
    QScopedPointer<ScenePage> scenePage(new ScenePage(*config_, GetContext()));
    if (scenePage->Open())
        mainWindow_->AddPage(scenePage.take());
}

//////////////////////////////////////////////////////////////////////////
ScenePage::ScenePage(Configuration& config, Urho3D::Context* context)
    : MainWindowPage(config)
    , Object(context)
    , cameraNode_(context_)
    , camera_(cameraNode_.CreateComponent<Urho3D::Camera>())
{
    cameraNode_.SetWorldPosition(Urho3D::Vector3(0, 1, -1));
    cameraNode_.LookAt(Urho3D::Vector3::ZERO);
}

void ScenePage::OnSelected()
{
    Urho3D::Renderer* renderer = GetContext()->GetSubsystem<Urho3D::Renderer>();
    renderer->SetViewport(0, viewport_);
}

bool ScenePage::DoLoad(const QString& fileName)
{
    Urho3D::File file(context_);
    if (!file.Open(fileName.toStdString().c_str()))
        return false;

    Urho3D::SharedPtr<Urho3D::Scene> scene(new Urho3D::Scene(context_));
    if (!scene->LoadXML(file))
        return false;

    scene_ = scene;
    viewport_ = new Urho3D::Viewport(context_, scene_, camera_);
    return true;
}

}
