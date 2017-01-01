#include "SceneEditor.h"
#include "Configuration.h"
#include "MainWindow.h"
#include <Urho3D/IO/File.h>
#include <Urho3D/Graphics/Renderer.h>

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
    menuFile->insertAction(menuFileNew_After, actionFileNewScene_.data());
    connect(actionFileNewScene_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleFileNewScene()));

    actionFileOpenScene_.reset(new QAction("Open Scene..."));
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
ScenePage::ScenePage(MainWindow& mainWindow)
    : MainWindowPage(mainWindow)
    , Object(mainWindow.GetContext())
    , cameraNode_(context_)
    , camera_(cameraNode_.CreateComponent<Urho3D::Camera>())
    , scene_(new Urho3D::Scene(context_))
{
    SetTitle("New Scene");
    cameraNode_.SetWorldPosition(Urho3D::Vector3(0, 1, -1));
    cameraNode_.LookAt(Urho3D::Vector3::ZERO);
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
    if (!file.Open(fileName.toStdString().c_str()))
        return false;

    if (!scene_->LoadXML(file))
        return false;

    viewport_ = new Urho3D::Viewport(context_, scene_, camera_);
    return true;
}

}
