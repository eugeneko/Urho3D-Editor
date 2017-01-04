#include "SceneEditor.h"
#include "Configuration.h"
#include "MainWindow.h"
#include "Bridge.h"
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

static const QString CONFIG_HOTKEY_MODE = "sceneeditor/blenderhotkeys";
static const QString CONFIG_DISABLE_DEBUG_RENDERER = "sceneeditor/debug/disable";
static const QString CONFIG_DISABLE_DEBUG_RENDERER_FOR_NODES_WITH_COMPONENTS = "sceneeditor/debug/disableforcomponents";
static const QString CONFIG_DEBUG_RENDERING = "sceneeditor/debug/rendering";
static const QString CONFIG_DEBUG_PHYSICS = "sceneeditor/debug/physics";
static const QString CONFIG_DEBUG_OCTREE = "sceneeditor/debug/octree";
static const QString CONFIG_DEBUG_NAVIGATION = "sceneeditor/debug/navigation";
static const QString CONFIG_PICK_MODE = "sceneeditor/pickmode";

SceneEditor::SceneEditor()
    : mainWindow_(nullptr)
{

}

bool SceneEditor::DoInitialize()
{
    mainWindow_ = GetModule<MainWindow>();
    if (!mainWindow_)
        return false;

    QMenuBar* menuBar = mainWindow_->GetMenuBar();
    QMenu* menuFile = mainWindow_->GetTopLevelMenu(MainWindow::MenuFile);
    QAction* menuFileNew_After = mainWindow_->GetMenuAction(MainWindow::MenuFileNew_After);
    QAction* menuFileOpen_After = mainWindow_->GetMenuAction(MainWindow::MenuFileOpen_After);
    QMenu* menuTools = mainWindow_->GetTopLevelMenu(MainWindow::MenuTools);
    if (!menuFile || !menuFileNew_After || !menuFileOpen_After || !menuTools)
        return false;

    connect(mainWindow_, SIGNAL(pageChanged(MainWindowPage*)), this, SLOT(HandleCurrentPageChanged(MainWindowPage*)));

    // Setup menu
    actionFileNewScene_.reset(new QAction("New Scene"));
    actionFileNewScene_->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_N);
    menuFile->insertAction(menuFileNew_After, actionFileNewScene_.data());
    connect(actionFileNewScene_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleFileNewScene()));

    actionFileOpenScene_.reset(new QAction("Open Scene..."));
    actionFileOpenScene_->setShortcut(Qt::CTRL + Qt::Key_O);
    menuFile->insertAction(menuFileOpen_After, actionFileOpenScene_.data());
    connect(actionFileOpenScene_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleFileOpenScene()));

    menuCreate_.reset(new QMenu("Create"));
    menuBar->insertMenu(menuTools->menuAction(), menuCreate_.data());

    actionCreateReplicatedNode_.reset(new QAction("Replicated Node"));
    menuCreate_->addAction(actionCreateReplicatedNode_.data());
    connect(actionCreateReplicatedNode_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleCreateReplicatedNode()));

    actionCreateLocalNode_.reset(new QAction("Local Node"));
    menuCreate_->addAction(actionCreateLocalNode_.data());
    connect(actionCreateLocalNode_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleCreateLocalNode()));

    UpdateMenuVisibility();

    // Setup config
    mainWindow_->GetConfig().SetDefault(CONFIG_HOTKEY_MODE, ScenePage::HotKeyStandard);
    mainWindow_->GetConfig().SetDefault(CONFIG_DISABLE_DEBUG_RENDERER, false);
    mainWindow_->GetConfig().SetDefault(CONFIG_DISABLE_DEBUG_RENDERER_FOR_NODES_WITH_COMPONENTS, QStringList("Terrain"));
    mainWindow_->GetConfig().SetDefault(CONFIG_DEBUG_RENDERING, false);
    mainWindow_->GetConfig().SetDefault(CONFIG_DEBUG_PHYSICS, false);
    mainWindow_->GetConfig().SetDefault(CONFIG_DEBUG_OCTREE, false);
    mainWindow_->GetConfig().SetDefault(CONFIG_DEBUG_NAVIGATION, false);
    mainWindow_->GetConfig().SetDefault(CONFIG_PICK_MODE, ScenePage::PickGeometries);

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

void SceneEditor::HandleCreateReplicatedNode()
{
    if (ScenePage* scenePage = dynamic_cast<ScenePage*>(mainWindow_->GetCurrentPage()))
    {

    }
}

void SceneEditor::HandleCreateLocalNode()
{
    if (ScenePage* scenePage = dynamic_cast<ScenePage*>(mainWindow_->GetCurrentPage()))
    {

    }
}

void SceneEditor::HandleCurrentPageChanged(MainWindowPage* page)
{
    UpdateMenuVisibility();
}

void SceneEditor::UpdateMenuVisibility()
{
    MainWindowPage* page = mainWindow_->GetCurrentPage();
    ScenePage* scenePage = dynamic_cast<ScenePage*>(page);
    menuCreate_->menuAction()->setVisible(!!scenePage);
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
    SubscribeToEvent(Urho3D::E_POSTRENDERUPDATE, URHO3D_HANDLER(ScenePage, HandlePostRenderUpdate));
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
    if (!IsActive())
        return;

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

void ScenePage::HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;

    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();
    const bool debugRendererDisabled = GetMainWindow().GetConfig().GetValue(CONFIG_DISABLE_DEBUG_RENDERER).toBool();
    if (debug && !debugRendererDisabled)
    {
        DrawDebugGeometry();
        DrawDebugComponents();
        PerformRaycast(false);
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

Urho3D::Ray ScenePage::GetCameraRay(const Urho3D::IntVector2& position) const
{
    if (Urho3D::View* view = viewport_->GetView())
    {
        const Urho3D::IntRect rect = view->GetViewRect();
        return camera_.GetCamera().GetScreenRay(
            float(position.x_ - rect.left_) / rect.Width(),
            float(position.y_ - rect.top_) / rect.Height());
    }
    else
        return Urho3D::Ray();
}

bool ScenePage::ShallDrawNodeDebug(Urho3D::Node* node)
{
    // Exception for the scene to avoid bringing the editor to its knees: drawing either the whole hierarchy or the subsystem-
    // components can have a large performance hit. Also skip nodes with some components.
    if (node == scene_)
        return false;

    const QStringList exceptionComponents =
        GetMainWindow().GetConfig().GetValue(CONFIG_DISABLE_DEBUG_RENDERER_FOR_NODES_WITH_COMPONENTS).toStringList();
    for (const QString& componentName : exceptionComponents)
        if (node->GetComponent(Cast(componentName)))
            return false;

    return true;
}

void ScenePage::DrawNodeDebug(Urho3D::Node* node, Urho3D::DebugRenderer* debug, bool drawNode /*= true*/)
{
    using namespace Urho3D;

    if (drawNode)
        debug->AddNode(node, 1.0, false);

    if (!ShallDrawNodeDebug(node))
        return;

    // Draw components
    const Vector<SharedPtr<Component>>& components = node->GetComponents();
    for (Component* component : components)
        component->DrawDebugGeometry(debug, false);

    // To avoid cluttering the view, do not draw the node axes for child nodes
    const Vector<SharedPtr<Node>>& children = node->GetChildren();
    for (Node* child : children)
        DrawNodeDebug(child, debug, false);
}

void ScenePage::DrawDebugGeometry()
{
    using namespace Urho3D;

    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();
    Renderer* renderer = GetSubsystem<Renderer>();

    // Visualize the currently selected nodes
    for (Node* node : selectedNodes_)
        DrawNodeDebug(node, debug);

    // Visualize the currently selected components
    for (Component* component : selectedComponents_)
        component->DrawDebugGeometry(debug, false);

    const bool debugRendering = GetMainWindow().GetConfig().GetValue(CONFIG_DEBUG_RENDERING).toBool();
    if (debugRendering)
        renderer->DrawDebugGeometry(false);
}

void ScenePage::DrawDebugComponents()
{
    using namespace Urho3D;

    Configuration& config = GetMainWindow().GetConfig();
    const bool debugPhysics = config.GetValue(CONFIG_DEBUG_PHYSICS).toBool();
    const bool debugOctree = config.GetValue(CONFIG_DEBUG_OCTREE).toBool();
    const bool debugNavigation = config.GetValue(CONFIG_DEBUG_NAVIGATION).toBool();

    PhysicsWorld* physicsWorld = scene_->GetComponent<PhysicsWorld>();
    Octree* octree = scene_->GetComponent<Octree>();
    CrowdManager* crowdManager = scene_->GetComponent<CrowdManager>();

    if (debugPhysics && physicsWorld)
        physicsWorld->DrawDebugGeometry(true);

    if (debugOctree && octree)
        octree->DrawDebugGeometry(true);

    if (debugNavigation && crowdManager)
    {
        crowdManager->DrawDebugGeometry(true);

        PODVector<NavigationMesh*> navMeshes;
        scene_->GetComponents(navMeshes, true);
        for (NavigationMesh* navMesh : navMeshes)
            navMesh->DrawDebugGeometry(true);

        PODVector<DynamicNavigationMesh*> dynNavMeshes;
        scene_->GetComponents(dynNavMeshes, true);
        for (DynamicNavigationMesh* dynNavMesh : dynNavMeshes)
            dynNavMesh->DrawDebugGeometry(true);
    }
}

void ScenePage::PerformRaycast(bool mouseClick)
{
    using namespace Urho3D;

    Input* input = GetSubsystem<Input>();
    if (input->IsMouseGrabbed())
        return;
    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();

    Ray cameraRay = GetCameraRay(input->GetMousePosition());
    Component* selectedComponent = nullptr;

    Configuration& config = GetMainWindow().GetConfig();
    HotKeyMode hotKeyMode = (HotKeyMode)config.GetValue(CONFIG_HOTKEY_MODE).toInt();

    PickMode pickMode = (PickMode)config.GetValue(CONFIG_PICK_MODE).toInt();
    if (pickMode < PickRigidbodies)
    {
        Octree* octree = scene_->GetComponent<Octree>();
        if (!octree)
            return;

        static int pickModeDrawableFlags[3] = { DRAWABLE_GEOMETRY, DRAWABLE_LIGHT, DRAWABLE_ZONE };
        PODVector<RayQueryResult> result;
        RayOctreeQuery query(result, cameraRay, RAY_TRIANGLE, camera_.GetCamera().GetFarClip(), pickModeDrawableFlags[pickMode], 0x7fffffff);
        octree->RaycastSingle(query);

        if (!result.Empty())
        {
            Drawable* drawable = result[0].drawable_;

            // for actual last selected node or component in both modes
            if (hotKeyMode == HotKeyStandard)
            {
                if (input->GetMouseButtonDown(MOUSEB_LEFT))
                {
                    // #TODO Fixme
//                     lastSelectedNode = drawable->GetNode();
//                     lastSelectedDrawable = drawable;
//                     lastSelectedComponent = drawable;
                }
            }
            else if (hotKeyMode == HotKeyBlender)
            {
                if (input->GetMouseButtonDown(MOUSEB_RIGHT))
                {
                    // #TODO Fixme
//                     lastSelectedNode = drawable->GetNode();
//                     lastSelectedDrawable = drawable;
//                     lastSelectedComponent = drawable;
                }
            }

            // If selecting a terrain patch, select the parent terrain instead
            if (drawable->GetTypeName() != "TerrainPatch")
            {
                selectedComponent = drawable;
                if (debug)
                {
                    debug->AddNode(drawable->GetNode(), 1.0, false);
                    drawable->DrawDebugGeometry(debug, false);
                }
            }
            else if (drawable->GetNode()->GetParent())
                selectedComponent = drawable->GetNode()->GetParent()->GetComponent("Terrain");
        }
    }
    else
    {
        PhysicsWorld* physicsWorld = scene_->GetComponent<PhysicsWorld>();
        if (!physicsWorld)
            return;

        // If we are not running the actual physics update, refresh collisions before raycasting
        //if (!runUpdate) #TODO Fixme
            physicsWorld->UpdateCollisions();

        PhysicsRaycastResult result;
        physicsWorld->RaycastSingle(result, cameraRay, camera_.GetCamera().GetFarClip());
        if (result.body_)
        {
            RigidBody* body = result.body_;
            if (debug)
            {
                debug->AddNode(body->GetNode(), 1.0, false);
                body->DrawDebugGeometry(debug, false);
            }
            selectedComponent = body;
        }
    }

    bool multiselect = false;
    bool componentSelectQualifier = false;
    bool mouseButtonPressRL = false;

    if (hotKeyMode == HotKeyStandard)
    {
        mouseButtonPressRL = input->GetMouseButtonPress(MOUSEB_LEFT);
        // #TODO It won't work
        componentSelectQualifier = input->GetQualifierDown(QUAL_SHIFT);
        multiselect = input->GetQualifierDown(QUAL_CTRL);
    }
    else if (hotKeyMode == HotKeyBlender)
    {
        mouseButtonPressRL = input->GetMouseButtonPress(MOUSEB_RIGHT);
        // #TODO It won't work
        componentSelectQualifier = input->GetQualifierDown(QUAL_CTRL);
        multiselect = input->GetQualifierDown(QUAL_SHIFT);
    }

    if (mouseClick && mouseButtonPressRL)
    {
        if (selectedComponent)
        {
            if (componentSelectQualifier)
            {
                // If we are selecting components, but have nodes in existing selection, do not multiselect to prevent confusion
//                 if (!selectedNodes.empty)
//                     multiselect = false;
//                 SelectComponent(selectedComponent, multiselect);
            }
            else
            {
                // If we are selecting nodes, but have components in existing selection, do not multiselect to prevent confusion
//                 if (!selectedComponents.empty)
//                     multiselect = false;
//                 SelectNode(selectedComponent->GetNode(), multiselect);
            }
        }
        else
        {
            // If clicked on emptiness in non-multiselect mode, clear the selection
//             if (!multiselect)
//                 SelectComponent(null, false);
        }
    }

}

}
