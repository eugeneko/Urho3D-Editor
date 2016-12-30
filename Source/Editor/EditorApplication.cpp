#include "EditorApplication.h"
#include "MainWindow.h"
#include "EditorSettings.h"
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
{
    context_->RegisterSubsystem(new EditorSettings(context_));
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

    mainWindow_.reset(new MainWindow(context_));
    mainWindow_->showMaximized();
    return exec();
}

void EditorApplication::AddDocument(AbstractDocument* document)
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
