#include "SceneEditor.h"
#include "SceneDocument.h"
#include "SceneOverlay.h"
#include "../Bridge.h"
#include "../Configuration.h"
#include "../MainWindow.h"
#include "../Widgets/Urho3DWidget.h"
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/OctreeQuery.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/View.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Navigation/CrowdManager.h>
#include <Urho3D/Navigation/DynamicNavigationMesh.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <QFileInfo>

namespace Urho3DEditor
{

static const QString CONFIG_HOTKEY_MODE = "sceneeditor/hotkeymode";
static const QString CONFIG_DISABLE_DEBUG_RENDERER = "sceneeditor/debug/disable";
static const QString CONFIG_DISABLE_DEBUG_RENDERER_FOR_NODES_WITH_COMPONENTS = "sceneeditor/debug/disableforcomponents";
static const QString CONFIG_DEBUG_RENDERING = "sceneeditor/debug/rendering";
static const QString CONFIG_DEBUG_PHYSICS = "sceneeditor/debug/physics";
static const QString CONFIG_DEBUG_OCTREE = "sceneeditor/debug/octree";
static const QString CONFIG_DEBUG_NAVIGATION = "sceneeditor/debug/navigation";
static const QString CONFIG_PICK_MODE = "sceneeditor/pickmode";

const QString SceneEditor::VarHotKeyMode = "scene/hotkey";

SceneEditor::SceneEditor()
{

}

bool SceneEditor::Initialize()
{
    MainWindow& mainWindow = GetMainWindow();
    connect(&mainWindow, SIGNAL(pageChanged(Document*)), this, SLOT(HandleCurrentPageChanged(Document*)));

    // Setup menu
    actionFileNewScene_.reset(mainWindow.AddAction("File.NewScene", Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    connect(actionFileNewScene_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleFileNewScene()));

    actionFileOpenScene_.reset(mainWindow.AddAction("File.OpenScene", Qt::CTRL + Qt::Key_O));
    connect(actionFileOpenScene_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleFileOpenScene()));

    actionCreateReplicatedNode_.reset(mainWindow.AddAction("Create.ReplicatedNode"));
    connect(actionCreateReplicatedNode_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleCreateReplicatedNode()));

    actionCreateLocalNode_.reset(mainWindow.AddAction("Create.LocalNode"));
    connect(actionCreateLocalNode_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleCreateLocalNode()));

    QAction* action = nullptr;

    mainWindow.AddAction("Scene.Camera.Single");
    mainWindow.AddAction("Scene.Camera.Vertical");
    mainWindow.AddAction("Scene.Camera.Horizontal");
    mainWindow.AddAction("Scene.Camera.Quad");
    mainWindow.AddAction("Scene.Camera.Top1_Bottom2");
    mainWindow.AddAction("Scene.Camera.Top2_Bottom1");
    mainWindow.AddAction("Scene.Camera.Left1_Right2");
    mainWindow.AddAction("Scene.Camera.Left2_Right1");

    UpdateMenuVisibility();

    // Setup config
    GetConfig().RegisterVariable(VarHotKeyMode, (int)HotKeyMode::Standard, "", "", QStringList({ "Standard", "Blender" }));
    GetConfig().RegisterVariable(CONFIG_DISABLE_DEBUG_RENDERER, false);
    GetConfig().RegisterVariable(CONFIG_DISABLE_DEBUG_RENDERER_FOR_NODES_WITH_COMPONENTS, QStringList("Terrain"));
    GetConfig().RegisterVariable(CONFIG_DEBUG_RENDERING, false);
    GetConfig().RegisterVariable(CONFIG_DEBUG_PHYSICS, false);
    GetConfig().RegisterVariable(CONFIG_DEBUG_OCTREE, false);
    GetConfig().RegisterVariable(CONFIG_DEBUG_NAVIGATION, false);
    GetConfig().RegisterVariable(CONFIG_PICK_MODE, SceneDocument::PickGeometries);

    return true;
}

void SceneEditor::HandleFileNewScene()
{
    MainWindow& mainWindow = GetMainWindow();
    mainWindow.AddPage(new SceneDocument(mainWindow));
}

void SceneEditor::HandleFileOpenScene()
{
    MainWindow& mainWindow = GetMainWindow();
    QScopedPointer<SceneDocument> scenePage(new SceneDocument(mainWindow));
    if (scenePage->Open())
        mainWindow.AddPage(scenePage.take());
}

void SceneEditor::HandleCreateReplicatedNode()
{
    MainWindow& mainWindow = GetMainWindow();
    if (SceneDocument* scenePage = dynamic_cast<SceneDocument*>(mainWindow.GetCurrentPage()))
    {

    }
}

void SceneEditor::HandleCreateLocalNode()
{
    MainWindow& mainWindow = GetMainWindow();
    if (SceneDocument* scenePage = dynamic_cast<SceneDocument*>(mainWindow.GetCurrentPage()))
    {

    }
}

void SceneEditor::HandleCurrentPageChanged(Document* document)
{
    UpdateMenuVisibility();
}

void SceneEditor::UpdateMenuVisibility()
{
    MainWindow& mainWindow = GetMainWindow();
    Document* document = mainWindow.GetCurrentPage();
    SceneDocument* sceneDocument = dynamic_cast<SceneDocument*>(document);
}

}
