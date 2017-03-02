#include "SceneDocument.h"
#include "SceneActions.h"
#include "SceneOverlay.h"
#include "SceneViewportManager.h"
#include "../Core/QtUrhoHelpers.h"
#include "../Configuration.h"
#include "../Core/Core.h"
#include "../Widgets/Urho3DWidget.h"
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Drawable.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <QFileInfo>
#include <QKeyEvent>

// #TODO Extract this code
#include "DebugRenderer.h"
#include "Gizmo.h"
#include "ObjectPicker.h"
#include <QMessageBox>

namespace Urho3DEditor
{

SceneDocument::SceneDocument(Core& core)
    : Document(core)
    , Object(core.GetUrho3DWidget().GetContext())
    , input_(*GetSubsystem<Urho3D::Input>())
    , core_(core.CreateUrho3DClientWidget(this))
    , wheelDelta_(0)
    , mouseMoveConsumed_(false)
    , scene_(new Urho3D::Scene(context_))
    , viewportManager_(new SceneViewportManager(*this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(core_);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    scene_->CreateComponent<Urho3D::Octree>();
    scene_->CreateComponent<Urho3D::DebugRenderer>();
    scene_->SetUpdateEnabled(false);

    AddOverlay(viewportManager_.data());
    SetTitle("New Scene");

    SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(SceneDocument, HandleUpdate));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(SceneDocument, HandleMouseButton));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONUP, URHO3D_HANDLER(SceneDocument, HandleMouseButton));
    SubscribeToEvent(Urho3D::E_POSTRENDERUPDATE, URHO3D_HANDLER(SceneDocument, HandlePostRenderUpdate));

    SubscribeToEvent(scene_, Urho3D::E_NODEREMOVED, URHO3D_HANDLER(SceneDocument, HandleNodeRemoved));
    SubscribeToEvent(scene_, Urho3D::E_COMPONENTREMOVED, URHO3D_HANDLER(SceneDocument, HandleComponentRemoved));

    connect(viewportManager_.data(), SIGNAL(viewportsChanged()), this, SLOT(HandleViewportsChanged()));

    Urho3DWidget& urhoWidget = core.GetUrho3DWidget();
    connect(&urhoWidget, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(HandleKeyPress(QKeyEvent*)));
    connect(&urhoWidget, SIGNAL(keyReleased(QKeyEvent*)), this, SLOT(HandleKeyRelease(QKeyEvent*)));
    connect(&urhoWidget, SIGNAL(wheelMoved(QWheelEvent*)), this, SLOT(HandleMouseWheel(QWheelEvent*)));
    connect(&urhoWidget, SIGNAL(focusOut()), this, SLOT(HandleFocusOut()));

    connect(core.GetAction("Edit.Cut"), SIGNAL(triggered(bool)), this, SLOT(Cut()));
    connect(core.GetAction("Edit.Duplicate"), SIGNAL(triggered(bool)), this, SLOT(Duplicate()));
    connect(core.GetAction("Edit.Copy"), SIGNAL(triggered(bool)), this, SLOT(Copy()));
    connect(core.GetAction("Edit.Paste"), SIGNAL(triggered(bool)), this, SLOT(Paste()));
    connect(core.GetAction("Edit.Delete"), SIGNAL(triggered(bool)), this, SLOT(Delete()));

    connect(core.GetAction("Scene.Camera.Single"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraSingle()));
    connect(core.GetAction("Scene.Camera.Vertical"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraVertical()));
    connect(core.GetAction("Scene.Camera.Horizontal"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraHorizontal()));
    connect(core.GetAction("Scene.Camera.Quad"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraQuad()));
    connect(core.GetAction("Scene.Camera.Top1_Bottom2"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraTop1Bottom2()));
    connect(core.GetAction("Scene.Camera.Top2_Bottom1"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraTop2Bottom1()));
    connect(core.GetAction("Scene.Camera.Left1_Right2"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraLeft1Right2()));
    connect(core.GetAction("Scene.Camera.Left2_Right1"), SIGNAL(triggered(bool)), this, SLOT(HandleCameraLeft2Right1()));

    // #TODO Extract this code
    Get<Gizmo, SceneDocument>();
    Get<ObjectPicker, SceneDocument>();
    Get<DebugRenderer, SceneDocument>();

}

SceneDocument::~SceneDocument()
{
    scene_.Reset();
}

void SceneDocument::Undo()
{
    undoStack_.undo();
}

void SceneDocument::Redo()
{
    undoStack_.redo();
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

void SceneDocument::AddAction(QUndoCommand* action)
{
    undoStack_.push(action);
}

Urho3D::Camera& SceneDocument::GetCurrentCamera()
{
    return viewportManager_->GetCurrentCamera();
}

void SceneDocument::SetSelection(const QSet<Urho3D::Object*>& objects)
{
    selectedObjects_ = objects;
    GatherSelection();
    emit selectionChanged();
}

void SceneDocument::ClearSelection()
{
    selectedObjects_.clear();
    GatherSelection();
    emit selectionChanged();
}

void SceneDocument::SelectObjects(const QSet<Urho3D::Object*>& objects)
{
    selectedObjects_ = objects;
    GatherSelection();
    emit selectionChanged();
}

void SceneDocument::SelectObject(Urho3D::Object* object, SelectionAction action, bool clearSelection)
{
    if (clearSelection)
        selectedObjects_.clear();

    const bool wasSelected = selectedObjects_.remove(object);
    if (!wasSelected && action != SelectionAction::Deselect)
        selectedObjects_.insert(object);

    GatherSelection();
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

//////////////////////////////////////////////////////////////////////////
bool SceneDocument::Cut()
{
    return Copy() && Delete();
}

bool SceneDocument::Duplicate()
{
    QVector<Urho3D::SharedPtr<Urho3D::XMLFile>> copy = copyBuffer_;
    const bool result = Copy() && Paste(true);
    copyBuffer_ = copy;
    return result;
}

bool SceneDocument::Copy()
{
    using namespace Urho3D;
    copyBuffer_.clear();

    // Copy components
    if (!GetSelectedComponents().empty())
    {
        for (Component* component : GetSelectedComponents())
        {
            SharedPtr<XMLFile> xml(new XMLFile(context_));
            XMLElement rootElem = xml->CreateRoot("component");
            component->SaveXML(rootElem);
            rootElem.SetBool("local", component->GetID() >= FIRST_LOCAL_ID);
            copyBuffer_.push_back(xml);
        }
    }
    // Copy nodes
    else
    {
        for (Node* node : GetSelectedNodes())
        {
            // Skip the root scene node as it cannot be copied
            if (node == scene_)
                continue;

            SharedPtr<XMLFile> xml(new XMLFile(context_));
            XMLElement rootElem = xml->CreateRoot("node");
            node->SaveXML(rootElem);
            rootElem.SetBool("local", node->GetID() >= FIRST_LOCAL_ID);
            copyBuffer_.push_back(xml);
        }
    }
    return true;
}

bool SceneDocument::Paste(bool duplication /*= false*/)
{
    using namespace Urho3D;

    // Group for storing undo actions
    QScopedPointer<QUndoCommand> group(new QUndoCommand);

    const NodeSet selectedNodes = GetSelectedNodesAndComponents();
    for (SharedPtr<XMLFile>& copyElement : copyBuffer_)
    {
        XMLElement rootElem = copyElement->GetRoot();
        String mode = rootElem.GetName();
        if (mode == "component" && !selectedNodes.empty())
        {
            for (Node* node : selectedNodes)
            {
                // If this is the root node, do not allow to create duplicate scene-global components
                if (node == scene_ && CheckForExistingGlobalComponent(*node, rootElem.GetAttribute("type")))
                    return false;

                // Create an undo action
                const unsigned componentId = scene_->GetFreeComponentID(rootElem.GetBool("local") ? LOCAL : REPLICATED);
                new CreateComponentAction(*this, copyElement, componentId, node->GetID(), group.data());
            }
        }
        else if (mode == "node")
        {
            Node* thisNode = scene_->GetNode(rootElem.GetUInt("id"));

            QList<Node*> destNodes;
            // Paste into scene if nothing selected
            if (selectedNodes.empty())
                destNodes.push_back(scene_);
            // Paste into parent if duplicate or selected single node and paste onto itself
            else if (duplication || copyBuffer_.size() == 1 && selectedNodes.contains(thisNode))
                destNodes.push_back(thisNode->GetParent() ? thisNode->GetParent() : scene_);
            // Paste into all selected nodes else
            else
                destNodes = selectedNodes.toList();

            // Create actions
            for (Node* destNode : destNodes)
            {
                const unsigned nodeId = scene_->GetFreeNodeID(rootElem.GetBool("local") ? LOCAL : REPLICATED);
                new CreateNodeAction(*this, copyElement, nodeId, destNode->GetID(), group.data());
            }
        }
    }

    AddAction(group.take());
    return true;
}

bool SceneDocument::Delete()
{
    using namespace Urho3D;

    // Group for storing undo actions
    QScopedPointer<QUndoCommand> group(new QUndoCommand);

    // Remove nodes
    const NodeSet selectedNodes = GetSelectedNodes();
    for (Node* node : selectedNodes)
    {
        if (!node->GetParent() || !node->GetScene())
            continue; // Root or already deleted

        // Create undo action
        new DeleteNodeAction(*this, *node, group.data());

        /// \todo If deleting only one node, select the next item in the same index
    }

    // Then remove components, if they still remain
    const ComponentSet selectedComponents = GetSelectedComponents();
    for (Component* component : selectedComponents)
    {
        Node* node = component->GetNode();
        if (!node)
            continue; // Already deleted

        // Do not allow to remove the Octree, DebugRenderer or MaterialCache2D or DrawableProxy2D from the root node
        if (node == scene_ && (component->GetTypeName() == "Octree" || component->GetTypeName() == "DebugRenderer" ||
            component->GetTypeName() == "MaterialCache2D" || component->GetTypeName() == "DrawableProxy2D"))
            continue;

        // Create undo action
        new DeleteComponentAction(*this, *component, group.data());

        /// \todo If deleting only one component, select the next item in the same index
    }

    AddAction(group.take());
    ClearSelection();
    return true;
}

//////////////////////////////////////////////////////////////////////////
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
        mouseButtonsConsumed_.clear();
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
    for (SceneOverlay* overlay : overlays_)
        overlay->PostRenderUpdate(*this);

    keysPressed_.clear();
    mouseButtonsPressed_.clear();
    mouseButtonsConsumed_.clear();
    wheelDelta_ = 0;
    mouseMoveConsumed_ = false;
}

void SceneDocument::HandleNodeRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;
    Object* object = dynamic_cast<Object*>(eventData[NodeRemoved::P_NODE].GetPtr());
    if (selectedObjects_.remove(object))
    {
        GatherSelection();
        emit selectionChanged();
    }
}

void SceneDocument::HandleComponentRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;
    Object* object = dynamic_cast<Object*>(eventData[ComponentRemoved::P_COMPONENT].GetPtr());
    if (selectedObjects_.remove(object))
    {
        GatherSelection();
        emit selectionChanged();
    }
}

void SceneDocument::HandleCurrentDocumentChanged(Document* document)
{
    if (IsActive())
    {
        core_->Acquire();
        viewportManager_->ApplyViewports();
    }
    else
    {
        keysDown_.clear();
        keysPressed_.clear();
        mouseButtonsDown_.clear();
        mouseButtonsPressed_.clear();
        mouseButtonsConsumed_.clear();
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

    scene_->SetUpdateEnabled(false);
    return true;
}

void SceneDocument::GatherSelection()
{
    using namespace Urho3D;
    selectedNodes_.clear();
    selectedComponents_.clear();
    selectedNodesAndComponents_.clear();
    for (Object* object : selectedObjects_)
    {
        if (Node* node = dynamic_cast<Node*>(object))
        {
            selectedNodes_.insert(node);
            selectedNodesAndComponents_.insert(node);
        }
        if (Component* component = dynamic_cast<Component*>(object))
        {
            selectedComponents_.insert(component);
            selectedNodesAndComponents_.insert(component->GetNode());
        }
    }
}

bool SceneDocument::CheckForExistingGlobalComponent(Urho3D::Node& node, const Urho3D::String& typeName)
{
    if (typeName != "Octree" && typeName != "PhysicsWorld" && typeName != "DebugRenderer")
        return false;
    else
        return node.HasComponent(typeName);
}

}
