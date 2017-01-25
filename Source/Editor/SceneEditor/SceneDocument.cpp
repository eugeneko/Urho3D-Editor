#include "SceneDocument.h"
#include "SceneOverlay.h"
#include "SceneViewportManager.h"
#include "../Bridge.h"
#include "../Configuration.h"
#include "../MainWindow.h"
#include "../Widgets/Urho3DWidget.h"
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Drawable.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Input/Input.h>
#include <QFileInfo>
#include <QKeyEvent>

// #TODO Extract this code
#include "DebugRenderer.h"
#include "Gizmo.h"
#include "ObjectPicker.h"

namespace Urho3DEditor
{

SceneDocument::SceneDocument(MainWindow& mainWindow)
    : Document(mainWindow)
    , Object(&mainWindow.GetContext())
    , input_(*GetSubsystem<Urho3D::Input>())
    , widget_(*mainWindow.GetUrho3DWidget())
    , wheelDelta_(0)
    , mouseMoveConsumed_(false)
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
    Get<DebugRenderer, SceneDocument>();

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

void SceneDocument::HandleCurrentDocumentChanged(Document* document)
{
    if (IsActive())
        viewportManager_->ApplyViewports();
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

}
