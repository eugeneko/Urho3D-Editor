#include "SceneDocument.h"
#include "SceneOverlay.h"
#include "SceneViewportManager.h"
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

// #TODO Extract this code
#include "Gizmo.h"
#include "ObjectPicker.h"

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
    , viewportManager_(new SceneViewportManager(*this))
{
    AddOverlay(viewportManager_.data());
    SetTitle("New Scene");

    SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(SceneDocument, HandleUpdate));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(SceneDocument, HandleMouseButton));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONUP, URHO3D_HANDLER(SceneDocument, HandleMouseButton));
    SubscribeToEvent(Urho3D::E_POSTRENDERUPDATE, URHO3D_HANDLER(SceneDocument, HandlePostRenderUpdate));

    connect(viewportManager_.data(), SIGNAL(viewportsChanged()), this, SLOT(HandleViewportsChanged()));
    connect(&widget_, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(HandleKeyPress(QKeyEvent*)));
    connect(&widget_, SIGNAL(keyReleased(QKeyEvent*)), this, SLOT(HandleKeyRelease(QKeyEvent*)));
    connect(&widget_, SIGNAL(wheelMoved(QWheelEvent*)), this, SLOT(HandleMouseWheel(QWheelEvent*)));
    connect(&widget_, SIGNAL(focusOut()), this, SLOT(HandleFocusOut()));

    connect(mainWindow.GetAction("Scene.Camera.Single"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraSingle()));
    connect(mainWindow.GetAction("Scene.Camera.Vertical"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraVertical()));
    connect(mainWindow.GetAction("Scene.Camera.Horizontal"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraHorizontal()));
    connect(mainWindow.GetAction("Scene.Camera.Quad"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraQuad()));
    connect(mainWindow.GetAction("Scene.Camera.Top1_Bottom2"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraTop1Bottom2()));
    connect(mainWindow.GetAction("Scene.Camera.Top2_Bottom1"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraTop2Bottom1()));
    connect(mainWindow.GetAction("Scene.Camera.Left1_Right2"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraLeft1Right2()));
    connect(mainWindow.GetAction("Scene.Camera.Left2_Right1"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraLeft2Right1()));

    // #TODO Extract this code
    Get<Gizmo, SceneDocument>();
    Get<ObjectPicker, SceneDocument>();

}

void SceneDocument::Undo()
{
    if (!undoActions_.empty())
    {
        ActionGroup group = undoActions_.takeLast();
        group.Undo();
        redoActions_.push_back(group);
    }
}

void SceneDocument::Redo()
{
    if (!redoActions_.empty())
    {
        ActionGroup group = redoActions_.takeLast();
        group.Redo();
        undoActions_.push_back(group);
    }
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
    undoActions_.push_back(actionGroup);
}

void SceneDocument::UndoAction()
{
    // #TODO Implement me
}

void SceneDocument::RedoAction()
{
    // #TODO Implement me
}

Urho3D::Camera& SceneDocument::GetCurrentCamera()
{
    return viewportManager_->GetCurrentCamera();
}

void SceneDocument::SetSelection(const NodeSet& selectedNodes, const ComponentSet& selectedComponents)
{
    selectedNodes_ = selectedNodes;
    selectedComponents_ = selectedComponents;
    GatherSelectedNodes();
    emit selectionChanged();
}

void SceneDocument::ClearSelection()
{
    selectedNodes_.clear();
    selectedComponents_.clear();
    GatherSelectedNodes();
    emit selectionChanged();
}

void SceneDocument::SelectNode(Urho3D::Node* node, SelectionAction action, bool clearSelection)
{
    if (clearSelection)
    {
        selectedNodes_.clear();
        selectedComponents_.clear();
    }

    const bool wasSelected = selectedNodes_.remove(node);
    if (!wasSelected && action != SelectionAction::Deselect)
        selectedNodes_.insert(node);

    GatherSelectedNodes();
    emit selectionChanged();
}

void SceneDocument::SelectComponent(Urho3D::Component* component, SelectionAction action, bool clearSelection)
{
    if (clearSelection)
    {
        selectedNodes_.clear();
        selectedComponents_.clear();
    }

    const bool wasSelected = selectedComponents_.remove(component);
    if (!wasSelected && action != SelectionAction::Deselect)
        selectedComponents_.insert(component);

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

Urho3D::IntVector2 SceneDocument::GetMousePosition() const
{
    return input_.GetMousePosition();
}

Urho3D::IntVector2 SceneDocument::GetMouseMove() const
{
    return input_.GetMouseMove();
}

Urho3D::Ray SceneDocument::GetMouseRay() const
{
    return viewportManager_->GetCurrentCameraRay();
}

QString SceneDocument::GetNameFilters()
{
    return "Urho3D Scene (*.xml *.json *.bin);;All files (*.*)";
}

void SceneDocument::HandleCameraSingle()
{
    viewportManager_->SetLayout(SceneViewportLayout::Single);
}

void SceneDocument::HandleCameraVertical()
{
    viewportManager_->SetLayout(SceneViewportLayout::Vertical);
}

void SceneDocument::HandleCameraHorizontal()
{
    viewportManager_->SetLayout(SceneViewportLayout::Horizontal);
}

void SceneDocument::HandleCameraQuad()
{
    viewportManager_->SetLayout(SceneViewportLayout::Quad);
}

void SceneDocument::HandleCameraTop1Bottom2()
{
    viewportManager_->SetLayout(SceneViewportLayout::Top1_Bottom2);
}

void SceneDocument::HandleCameraTop2Bottom1()
{
    viewportManager_->SetLayout(SceneViewportLayout::Top2_Bottom1);
}

void SceneDocument::HandleCameraLeft1Right2()
{
    viewportManager_->SetLayout(SceneViewportLayout::Left1_Right2);
}

void SceneDocument::HandleCameraLeft2Right1()
{
    viewportManager_->SetLayout(SceneViewportLayout::Left2_Right1);
}

void SceneDocument::HandleViewportsChanged()
{
    if (IsActive())
        viewportManager_->ApplyViewports();
}

void SceneDocument::HandleKeyPress(QKeyEvent* event)
{
    if (IsActive())
    {
        keysPressed_.insert((Qt::Key)event->key());
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
        keysPressed_.clear();
        mouseButtonsDown_.clear();
        mouseButtonsPressed_.clear();
    }
}

void SceneDocument::HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    if (!IsActive())
        return;

    const float timeStep = eventData[Urho3D::Update::P_TIMESTEP].GetFloat();
    for (SceneOverlay* overlay : overlays_)
        overlay->Update(*this, timeStep);
}

void SceneDocument::HandleMouseButton(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    if (!IsActive())
        return;

    using namespace Urho3D;
    const Qt::MouseButton button = ConvertMouseButton(eventData[MouseButtonDown::P_BUTTON].GetInt());
    const bool pressed = eventType == E_MOUSEBUTTONDOWN;

    if (pressed)
    {
        mouseButtonsPressed_.insert(button);
        mouseButtonsDown_.insert(button);
    }
    else
        mouseButtonsDown_.remove(button);
}

void SceneDocument::HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;

    for (SceneOverlay* overlay : overlays_)
        overlay->PostRenderUpdate(*this);

    keysPressed_.clear();
    mouseButtonsPressed_.clear();
    wheelDelta_ = 0;

    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();
    const bool debugRendererDisabled = GetMainWindow().GetConfig().GetValue(CONFIG_DISABLE_DEBUG_RENDERER).toBool();
    if (debug && !debugRendererDisabled)
    {
        DrawDebugGeometry();
        DrawDebugComponents();
    }
}

void SceneDocument::HandleCurrentPageChanged(Document* document)
{
    if (IsActive())
        viewportManager_->ApplyViewports();
    else
    {
        keysDown_.clear();
        keysPressed_.clear();
        mouseButtonsDown_.clear();
        mouseButtonsPressed_.clear();
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

void SceneDocument::GatherSelectedNodes()
{
    selectedNodesCombined_ = selectedNodes_;
    for (Urho3D::Component* component : selectedComponents_)
        selectedNodesCombined_.insert(component->GetNode());
}

}
