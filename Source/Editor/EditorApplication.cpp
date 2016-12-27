#include "EditorApplication.h"
#include "MainWindow.h"
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <QFile>
#include <QHBoxLayout>
#include <QTabBar>
#include <QTimer>

#include <Urho3D/Urho3DAll.h>

namespace Urho3D
{

EditorApplication::EditorApplication(int argc, char** argv, Urho3D::Context* context)
    : QApplication(argc, argv)
    , context_(context)
    , activeDirectory_(GetArguments().Size() > 0 ? GetArguments()[0].CString() : ".")
    , mainWindow_(new MainWindow(context))
    , engine_(MakeShared<Engine>(context))
{
}

EditorApplication::~EditorApplication()
{
}

void EditorApplication::AddPlugin(SharedPtr<EditorPlugin> plugin)
{
    plugins_.Push(plugin);
}

int EditorApplication::Run()
{
    // Setup style
    QFile file(activeDirectory_ + "/qdarkstyle/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
        setStyleSheet(QLatin1String(file.readAll()));

    mainWindow_->CreateWidgets();

    VariantMap engineParameters = Engine::ParseParameters(GetArguments());
    engineParameters[EP_FULL_SCREEN] = false;
    engineParameters[EP_BORDERLESS] = true;
    engineParameters[EP_WINDOW_RESIZABLE] = true;
    engineParameters[EP_EXTERNAL_WINDOW] = (void*)mainWindow_->GetClientWidget()->winId();

    if (!engine_->Initialize(engineParameters))
        return -1;

    QTimer timer;
    connect(&timer, SIGNAL(timeout()), this, SLOT(HandleTimer()));
    timer.start(16);

    //////////////////////////////////////////////////////////////////////////
    Scene* scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();

    // Create camera.
    Node* cameraNode_ = new Node(context_);
    Camera* camera = cameraNode_->CreateComponent<Camera>();

    camera->SetOrthographic(true);
    Graphics* graphic = context_->GetSubsystem<Graphics>();
    camera->SetOrthoSize(graphic->GetHeight() * PIXEL_SIZE);

    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera));

    Renderer* renderer = context_->GetSubsystem<Renderer>();
    renderer->SetViewport(0, viewport);
    //////////////////////////////////////////////////////////////////////////

    mainWindow_->showMaximized();
    return exec();
}

void EditorApplication::AddDocument(EditorDocument* document)
{

}

QMenu* EditorApplication::GetMainMenu(const String& name, const String& beforeName)
{
//     if (QMenu* menu = FindMainMenu(name.CString()))
//         return menu;
//     QMenu* beforeMenu = beforeName.Empty() ? nullptr : FindMainMenu(beforeName.CString());
//     QMenu* newMenu = new QMenu(name.CString(), mainWindow_.data());
//     mainMenu_->insertMenu(beforeMenu ? beforeMenu->menuAction() : nullptr, newMenu);
//     return newMenu;
    return nullptr;
}

void EditorApplication::HandleTimer()
{
    if (engine_ && !engine_->IsExiting())
        engine_->RunFrame();
}

QMenu* EditorApplication::FindMainMenu(const QString& name)
{
//     for (QObject* item : mainMenu_->children())
//         if (QMenu* menu = dynamic_cast<QMenu*>(item))
//             if (menu->title() == name)
//                 return menu;
    return nullptr;
}

void SceneEditorPlugin::Register(EditorInterface& editor)
{
    QMenu* menuFile = editor.GetMainMenu("File");
    QAction* menuNewScene = menuFile->addAction("New Scene");
    connect(menuNewScene, SIGNAL(triggered()), this, SLOT(HandleNewScene()));
}

void SceneEditorPlugin::Unregister(EditorInterface& editor)
{

}

void SceneEditorPlugin::HandleNewScene()
{

}

}
