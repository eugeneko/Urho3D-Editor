#include "SceneEditor.h"
#include "Gizmo.h"
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

const QString SceneEditor::VarHotKeyMode =                 "scene.camera/hotkey";
const QString SceneEditor::VarCameraBaseSpeed =            "scene.camera/speedbase";
const QString SceneEditor::VarCameraShiftSpeedMultiplier = "scene.camera/shiftfactor";
const QString SceneEditor::VarCameraBaseRotationSpeed =    "scene.camera/speedrotation";
const QString SceneEditor::VarMouseWheelCameraPosition =   "scene.camera/wheelposition";
const QString SceneEditor::VarMmbPanMode =                 "scene.camera/mmbpan";
const QString SceneEditor::VarLimitRotation =              "scene.camera/limitrot";

const QString SceneEditor::VarGizmoType =     "scene.gizmo/type";
const QString SceneEditor::VarGizmoAxisMode = "scene.gizmo/axismode";

const QString SceneEditor::VarSnapFactor =       "scene.gizmo/snap.factor";
const QString SceneEditor::VarSnapPosition =     "scene.gizmo/snap.position";
const QString SceneEditor::VarSnapRotation =     "scene.gizmo/snap.rotation";
const QString SceneEditor::VarSnapScale =        "scene.gizmo/snap.scale";
const QString SceneEditor::VarSnapPositionStep = "scene.gizmo/step.position";
const QString SceneEditor::VarSnapRotationStep = "scene.gizmo/step.rotation";
const QString SceneEditor::VarSnapScaleStep =    "scene.gizmo/step.scale";

const QString SceneEditor::VarModelPosition = "scene.gizmo/model.position";
const QString SceneEditor::VarModelRotation = "scene.gizmo/model.rotation";
const QString SceneEditor::VarModelScale =    "scene.gizmo/model.scale";

const QString SceneEditor::VarMaterialRed =   "scene.gizmo/material.red";
const QString SceneEditor::VarMaterialGreen = "scene.gizmo/material.green";
const QString SceneEditor::VarMaterialBlue =  "scene.gizmo/material.blue";

const QString SceneEditor::VarMaterialRedHighlight =   "scene.gizmo/material.red.h";
const QString SceneEditor::VarMaterialGreenHighlight = "scene.gizmo/material.green.h";
const QString SceneEditor::VarMaterialBlueHighlight =  "scene.gizmo/material.blue.h";

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
    Configuration& config = GetConfig();
    const QStringList hotkeyEnums = { "Standard", "Blender" };
    const QStringList gizmoTypeEnums = { "Position", "Rotation", "Scale", "Select" };
    const QStringList gizmoAxisModeEnums = { "Local", "World" };

    config.RegisterVariable(VarHotKeyMode, (int)HotKeyMode::Standard, "Scene.Camera", "HotKey Mode", hotkeyEnums);
    config.RegisterVariable(VarCameraBaseSpeed, 5.0, "Scene.Camera", "Camera Speed");
    config.RegisterVariable(VarCameraShiftSpeedMultiplier, 5.0, "Scene.Camera", "Shift Speed Multiplier");
    config.RegisterVariable(VarCameraBaseRotationSpeed, 0.2, "Scene.Camera", "Rotation Speed");
    config.RegisterVariable(VarMouseWheelCameraPosition, true, "Scene.Camera", "Mouse Wheel controls Camera Position");
    config.RegisterVariable(VarMmbPanMode, true, "Scene.Camera", "Mouse Middle Button makes Camera pan");
    config.RegisterVariable(VarLimitRotation, true, "Scene.Camera", "Limit Camera Rotation");

    config.RegisterVariable(VarGizmoType,     (int)GizmoType::Position,  "Scene.Gizmo", "Type", gizmoTypeEnums);
    config.RegisterVariable(VarGizmoAxisMode, (int)GizmoAxisMode::Local, "Scene.Gizmo", "Axis Mode", gizmoAxisModeEnums);

    config.RegisterVariable(VarSnapFactor,       1.0,   "Scene.Gizmo", "Snap Factor");
    config.RegisterVariable(VarSnapPosition,     false, "Scene.Gizmo", "Snap Position");
    config.RegisterVariable(VarSnapRotation,     false, "Scene.Gizmo", "Snap Rotation");
    config.RegisterVariable(VarSnapScale,        false, "Scene.Gizmo", "Snap Scale");
    config.RegisterVariable(VarSnapPositionStep, 0.5,   "Scene.Gizmo", "Position Step");
    config.RegisterVariable(VarSnapRotationStep, 5.0,   "Scene.Gizmo", "Rotation Step");
    config.RegisterVariable(VarSnapScaleStep,    1.0,   "Scene.Gizmo", "Scale Step");

    config.RegisterVariable(VarModelPosition, "Models/Editor/Axes.mdl",       "Scene.Gizmo", "Model Position");
    config.RegisterVariable(VarModelRotation, "Models/Editor/RotateAxes.mdl", "Scene.Gizmo", "Model Rotation");
    config.RegisterVariable(VarModelScale,    "Models/Editor/ScaleAxes.mdl",  "Scene.Gizmo", "Model Scale");

    config.RegisterVariable(VarMaterialRed,   "Materials/Editor/RedUnlit.xml",   "Scene.Gizmo", "Material Red");
    config.RegisterVariable(VarMaterialGreen, "Materials/Editor/GreenUnlit.xml", "Scene.Gizmo", "Material Green");
    config.RegisterVariable(VarMaterialBlue,  "Materials/Editor/BlueUnlit.xml",  "Scene.Gizmo", "Material Blue");

    config.RegisterVariable(VarMaterialRedHighlight,   "Materials/Editor/BrightRedUnlit.xml",   "Scene.Gizmo", "Material Red (Highlight)");
    config.RegisterVariable(VarMaterialGreenHighlight, "Materials/Editor/BrightGreenUnlit.xml", "Scene.Gizmo", "Material Green (Highlight)");
    config.RegisterVariable(VarMaterialBlueHighlight,  "Materials/Editor/BrightBlueUnlit.xml",  "Scene.Gizmo", "Material Blue (Highlight)");


    config.RegisterVariable(CONFIG_DISABLE_DEBUG_RENDERER, false);
    config.RegisterVariable(CONFIG_DISABLE_DEBUG_RENDERER_FOR_NODES_WITH_COMPONENTS, QStringList("Terrain"));
    config.RegisterVariable(CONFIG_DEBUG_RENDERING, false);
    config.RegisterVariable(CONFIG_DEBUG_PHYSICS, false);
    config.RegisterVariable(CONFIG_DEBUG_OCTREE, false);
    config.RegisterVariable(CONFIG_DEBUG_NAVIGATION, false);
    config.RegisterVariable(CONFIG_PICK_MODE, SceneDocument::PickGeometries);

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
