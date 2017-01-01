#include "Application.h"
#include "MainWindow.h"
#include "Urho3DProject.h"
#include "SceneEditor.h"
#include "Configuration.h"
#include "EditorSettings.h"
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <QFile>
#include <QHBoxLayout>
#include <QTabBar>
#include <QTimer>

#include <Urho3D/Urho3DAll.h>

namespace Urho3DEditor
{

Application::Application(int argc, char** argv, Urho3D::Context* context)
    : QApplication(argc, argv)
    , context_(context)
    , activeDirectory_(GetArguments().Size() > 0 ? GetArguments()[0].CString() : ".")
    , moduleSystem_(context_)
{
    context_->RegisterSubsystem(new EditorSettings(context_));
}

Application::~Application()
{
}

int Application::Run()
{
    // Setup style
    QFile file(activeDirectory_ + "/qdarkstyle/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
        setStyleSheet(QLatin1String(file.readAll()));

    // Create main window
    mainWindow_.reset(new QMainWindow());

    // Initialize modules
    if (!InitializeModules())
        return false;

    // Run!
    mainWindow_->showMaximized();
    return exec();
}

bool Application::InitializeModules()
{
    moduleSystem_.AddModule(new Configuration());
    moduleSystem_.AddModule(new MainWindow(mainWindow_.data(), context_));
    moduleSystem_.AddModule(new ProjectManager());
    moduleSystem_.AddModule(new SceneEditor());
    return true;
}

// void SceneEditorPlugin::Register(EditorInterface& editor)
// {
//     QMenu* menuFile = editor.GetMainMenu("File");
//     QAction* menuNewScene = menuFile->addAction("New Scene");
//     connect(menuNewScene, SIGNAL(triggered()), this, SLOT(HandleNewScene()));
// }
// 
// void SceneEditorPlugin::Unregister(EditorInterface& editor)
// {
// 
// }
// 
// void SceneEditorPlugin::HandleNewScene()
// {
// 
// }

}
