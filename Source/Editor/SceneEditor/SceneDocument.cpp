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
#include <QKeyEvent>

namespace Urho3DEditor
{

// #TODO Remove copy-paste
static const QString CONFIG_HOTKEY_MODE = "sceneeditor/hotkeymode";
static const QString CONFIG_DISABLE_DEBUG_RENDERER = "sceneeditor/debug/disable";
static const QString CONFIG_DISABLE_DEBUG_RENDERER_FOR_NODES_WITH_COMPONENTS = "sceneeditor/debug/disableforcomponents";
static const QString CONFIG_DEBUG_RENDERING = "sceneeditor/debug/rendering";
static const QString CONFIG_DEBUG_PHYSICS = "sceneeditor/debug/physics";
static const QString CONFIG_DEBUG_OCTREE = "sceneeditor/debug/octree";
static const QString CONFIG_DEBUG_NAVIGATION = "sceneeditor/debug/navigation";
static const QString CONFIG_PICK_MODE = "sceneeditor/pickmode";

SceneDocument::SceneDocument(MainWindow& mainWindow)
    : Document(mainWindow)
    , Object(&mainWindow.GetContext())
    , input_(*GetSubsystem<Urho3D::Input>())
    , widget_(*mainWindow.GetUrho3DWidget())
    , wheelDelta_(0)
    , scene_(new Urho3D::Scene(context_))
    , viewportManager_(*this)
{
    AddOverlay(&viewportManager_);
    SetTitle("New Scene");

    SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(SceneDocument, HandleUpdate));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(SceneDocument, HandleMouseButton));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONUP, URHO3D_HANDLER(SceneDocument, HandleMouseButton));
    SubscribeToEvent(Urho3D::E_POSTRENDERUPDATE, URHO3D_HANDLER(SceneDocument, HandlePostRenderUpdate));

    connect(&viewportManager_, SIGNAL(viewportsChanged()), this, SLOT(HandleViewportsChanged()));
    connect(&widget_, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(HandleKeyPress(QKeyEvent*)));
    connect(&widget_, SIGNAL(keyReleased(QKeyEvent*)), this, SLOT(HandleKeyRelease(QKeyEvent*)));
    connect(&widget_, SIGNAL(wheelMoved(QWheelEvent*)), this, SLOT(HandleMouseWheel(QWheelEvent*)));
    connect(&widget_, SIGNAL(focusOut()), this, SLOT(HandleFocusOut()));
}

void SceneDocument::AddOverlay(SceneOverlay* overlay)
{
    if (!overlays_.contains(overlay))
        overlays_.push_front(overlay);
}

void SceneDocument::RemoveOverlay(SceneOverlay* overlay)
{
    overlays_.removeAll(overlay);
}

void SceneDocument::AddAction(const ActionGroup& actionGroup)
{
    actions_.push_back(actionGroup);
}

void SceneDocument::UndoAction()
{
    // #TODO Implement me
}

void SceneDocument::RedoAction()
{
    // #TODO Implement me
}

void SceneDocument::SetSelection(const NodeSet& selectedNodes, const ComponentSet& selectedComponents)
{
    selectedNodes_ = selectedNodes;
    selectedComponents_ = selectedComponents;
    GatherSelectedNodes();
    emit selectionChanged();
}

Urho3D::Vector3 SceneDocument::GetSelectedCenter()
{
    using namespace Urho3D;

    const unsigned count = selectedNodes_.size() + selectedComponents_.size();
    Vector3 centerPoint;
    for (Node* node : selectedNodes_)
        centerPoint += node->GetWorldPosition();

    for (Component* component : selectedComponents_)
    {
        Drawable* drawable = dynamic_cast<Drawable*>(component);
        if (drawable)
            centerPoint += drawable->GetNode()->LocalToWorld(drawable->GetBoundingBox().Center());
        else
            centerPoint += component->GetNode()->GetWorldPosition();
    }

    if (count > 0)
        lastSelectedCenter_ = centerPoint / count;
    return lastSelectedCenter_;
}

void SceneDocument::SetMouseMode(Urho3D::MouseMode mouseMode)
{
    input_.SetMouseMode(mouseMode);
}

Urho3D::IntVector2 SceneDocument::GetMouseMove() const
{
    return input_.GetMouseMove();
}

QString SceneDocument::GetNameFilters()
{
    return "Urho3D Scene (*.xml *.json *.bin);;All files (*.*)";
}

void SceneDocument::HandleViewportsChanged()
{
    if (IsActive())
        viewportManager_.ApplyViewports();
}

void SceneDocument::HandleKeyPress(QKeyEvent* event)
{
    if (IsActive())
    {
        pressedKeys_.insert((Qt::Key)event->key());
        if (!event->isAutoRepeat())
            keysDown_.insert((Qt::Key)event->key());
    }
}

void SceneDocument::HandleKeyRelease(QKeyEvent* event)
{
    if (IsActive())
    {
        if (!event->isAutoRepeat())
            keysDown_.remove((Qt::Key)event->key());
    }
}

void SceneDocument::HandleMouseWheel(QWheelEvent* event)
{
    wheelDelta_ += event->delta() / 120;
}

void SceneDocument::HandleFocusOut()
{
    if (IsActive())
    {
        keysDown_.clear();
        pressedKeys_.clear();
    }
}

void SceneDocument::HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    if (!IsActive())
        return;

    const float timeStep = eventData[Urho3D::Update::P_TIMESTEP].GetFloat();
    const Urho3D::Ray ray = GetCameraRay(input_.GetMousePosition());

    for (SceneOverlay* overlay : overlays_)
        overlay->Update(*this, ray, timeStep);
}

void SceneDocument::HandleMouseButton(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    if (!IsActive())
        return;

    using namespace Urho3D;
    const int button = eventData[MouseButtonDown::P_BUTTON].GetInt();
    const bool pressed = eventType == E_MOUSEBUTTONDOWN;

    if (pressed)
        mouseButtonsDown_.insert(ConvertMouseButton(button));
    else
        mouseButtonsDown_.remove(ConvertMouseButton(button));
}

void SceneDocument::HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;

    const Urho3D::Ray ray = GetCameraRay(input_.GetMousePosition());
    for (SceneOverlay* overlay : overlays_)
        overlay->PostRenderUpdate(*this, ray);

    pressedKeys_.clear();
    wheelDelta_ = 0;

    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();
    const bool debugRendererDisabled = GetMainWindow().GetConfig().GetValue(CONFIG_DISABLE_DEBUG_RENDERER).toBool();
    if (debug && !debugRendererDisabled)
    {
        DrawDebugGeometry();
        DrawDebugComponents();
        PerformRaycast(false);
    }
}

void SceneDocument::HandleCurrentPageChanged(Document* document)
{
    if (IsActive())
        viewportManager_.ApplyViewports();
    else
    {
        keysDown_.clear();
        pressedKeys_.clear();
    }
}

bool SceneDocument::DoLoad(const QString& fileName)
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

Urho3D::Ray SceneDocument::GetCameraRay(const Urho3D::IntVector2& position) const
{
//     if (Urho3D::View* view = viewport_->GetView())
//     {
//         const Urho3D::IntRect rect = view->GetViewRect();
//         return camera_.GetCamera().GetScreenRay(
//             float(position.x_ - rect.left_) / rect.Width(),
//             float(position.y_ - rect.top_) / rect.Height());
//     }
//     else
        return Urho3D::Ray();
}

bool SceneDocument::ShallDrawNodeDebug(Urho3D::Node* node)
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

void SceneDocument::DrawNodeDebug(Urho3D::Node* node, Urho3D::DebugRenderer* debug, bool drawNode /*= true*/)
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

void SceneDocument::DrawDebugGeometry()
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

void SceneDocument::DrawDebugComponents()
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

void SceneDocument::PerformRaycast(bool mouseClick)
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
//         RayOctreeQuery query(result, cameraRay, RAY_TRIANGLE, camera_.GetCamera().GetFarClip(), pickModeDrawableFlags[pickMode], 0x7fffffff);
//         octree->RaycastSingle(query);

        if (!result.Empty())
        {
            Drawable* drawable = result[0].drawable_;

            // for actual last selected node or component in both modes
//             if (hotKeyMode == HotKeyStandard)
            {
                if (input->GetMouseButtonDown(MOUSEB_LEFT))
                {
                    // #TODO Fixme
//                     lastSelectedNode = drawable->GetNode();
//                     lastSelectedDrawable = drawable;
//                     lastSelectedComponent = drawable;
                }
            }
//             else if (hotKeyMode == HotKeyBlender)
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
//         physicsWorld->RaycastSingle(result, cameraRay, camera_.GetCamera().GetFarClip());
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

//     if (hotKeyMode == HotKeyStandard)
    {
        mouseButtonPressRL = input->GetMouseButtonPress(MOUSEB_LEFT);
        // #TODO It won't work
        componentSelectQualifier = input->GetQualifierDown(QUAL_SHIFT);
        multiselect = input->GetQualifierDown(QUAL_CTRL);
    }
//     else if (hotKeyMode == HotKeyBlender)
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

void SceneDocument::GatherSelectedNodes()
{
    selectedNodesCombined_ = selectedNodes_;
    for (Urho3D::Component* component : selectedComponents_)
        selectedNodesCombined_.insert(component->GetNode());
}

}
