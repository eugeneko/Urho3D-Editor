#pragma once

#include "SceneOverlay.h"
#include "../Document.h"
#include <QAction>
#include <QSet>
#include <QUndoStack>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

class Input;

}

namespace Urho3DEditor
{

class MainWindow;
class SceneOverlay;
class Urho3DWidget;
class SceneViewportManager;

/// Hot key mode. Affects camera, gizmo and selection control.
enum class HotKeyMode
{
    /// Standard mode.
    Standard,
    /// Blender mode.
    Blender
};

/// Selection action.
enum class SelectionAction
{
    /// Select object.
    Select,
    /// Deselect object.
    Deselect,
    /// Flip selection.
    Flip
};

/// Scene document.
class SceneDocument : public Document, public Urho3D::Object, public SceneInputInterface
{
    Q_OBJECT
    URHO3D_OBJECT(SceneDocument, Urho3D::Object);

public:
    /// Set of nodes.
    using NodeSet = QSet<Urho3D::Node*>;
    /// Set of components.
    using ComponentSet = QSet<Urho3D::Component*>;

public:
    /// Construct.
    SceneDocument(MainWindow& mainWindow);
    /// Destruct.
    virtual ~SceneDocument();
    /// Get scene.
    Urho3D::Scene& GetScene() const { return *scene_; }

    /// Undo.
    virtual void Undo() override;
    /// Redo.
    virtual void Redo() override;

    /// Add overlay.
    void AddOverlay(SceneOverlay* overlay);
    /// Remove overlay.
    void RemoveOverlay(SceneOverlay* overlay);

    /// Add action.
    void AddAction(QUndoCommand* action);

    /// Get current camera.
    Urho3D::Camera& GetCurrentCamera();

    /// Set selection.
    void SetSelection(const QSet<Urho3D::Object*>& objects);
    /// Clear selection.
    void ClearSelection();
    /// Select objects.
    void SelectObjects(const QSet<Urho3D::Object*>& objects);
    /// Select object.
    void SelectObject(Urho3D::Object* object, SelectionAction action, bool clearSelection);
    /// Get selected objects.
    const QSet<Urho3D::Object*>& GetSelected() const { return selectedObjects_; }
    /// Get selected nodes.
    const NodeSet& GetSelectedNodes() const { return selectedNodes_; }
    /// Get selected components.
    const ComponentSet& GetSelectedComponents() const { return selectedComponents_; }
    /// Get selected nodes and components.
    const NodeSet& GetSelectedNodesAndComponents() const { return selectedNodesAndComponents_; }
    /// Returns whether there are selected nodes and/or components.
    bool HasSelectedNodesOrComponents() const { return !selectedNodesAndComponents_.empty(); }
    /// Get center point of selected nodes.
    Urho3D::Vector3 GetSelectedCenter();

    // SceneInputInterface implementation
    // {

    /// Set mouse mode.
    virtual void SetMouseMode(Urho3D::MouseMode mouseMode) override;
    /// Return whether the key is down.
    virtual bool IsKeyDown(Qt::Key key) const override { return keysDown_.contains(key); }
    /// Return whether the key is pressed.
    virtual bool IsKeyPressed(Qt::Key key) const override { return keysPressed_.contains(key); }
    /// Return whether the mouse button is down.
    virtual bool IsMouseButtonDown(Qt::MouseButton mouseButton) const override { return mouseButtonsDown_.contains(mouseButton); }
    /// Return whether the mouse button is pressed.
    virtual bool IsMouseButtonPressed(Qt::MouseButton mouseButton) const override { return mouseButtonsPressed_.contains(mouseButton); }
    /// Return mouse position.
    virtual Urho3D::IntVector2 GetMousePosition() const override;
    /// Return mouse move.
    virtual Urho3D::IntVector2 GetMouseMove() const override;
    /// Return mouse wheel delta.
    virtual int GetMouseWheelMove() const override { return wheelDelta_; }
    /// Return mouse ray in 3D.
    virtual Urho3D::Ray GetMouseRay() const override;

    /// Consume mouse button input.
    virtual void ConsumeMouseButton(Qt::MouseButton mouseButton) override { mouseButtonsConsumed_.insert(mouseButton); }
    /// Checks whether mouse button is consumed.
    virtual bool IsMouseButtonConsumed(Qt::MouseButton mouseButton) const override { return mouseButtonsConsumed_.contains(mouseButton); }
    /// Consume mouse move.
    virtual void ConsumeMouseMove() override { mouseMoveConsumed_ = true; }
    /// Checks whether mouse move is consumed.
    virtual bool IsMouseMoveConsumed() const override { return mouseMoveConsumed_; }

    // @}

    /// Return title of the document.
    virtual QString GetTitle() override { return GetRawTitle(); }
    /// Return whether the document can be saved.
    virtual bool CanBeSaved() override { return true; }
    /// Return whether the document widget should be visible when the document is active.
    virtual bool IsDocumentWidgetVisible() override { return false; }
    /// Return whether the Urho3D widget should be visible when the document is active.
    virtual bool IsUrho3DWidgetVisible() override { return true; }
    /// Get name filters for open and save dialogs.
    virtual QString GetNameFilters() override;

signals:
    /// Signals that selection has been changed.
    void selectionChanged();
    /// Signals that node transforms has been changed.
    void nodeTransformChanged(const Urho3D::Node& node);
    /// Signals that component has been re-ordered.
    void componentReordered(const Urho3D::Component& component, unsigned index);

private slots:
    /// Cut.
    bool Cut();
    /// Duplicate.
    bool Duplicate();
    /// Copy.
    bool Copy();
    /// Paste.
    bool Paste(bool duplication = false);
    /// Delete.
    bool Delete();

    /// Handle 'Scene.Camera.Single'
    void HandleCameraSingle();
    /// Handle 'Scene.Camera.Vertical'
    void HandleCameraVertical();
    /// Handle 'Scene.Camera.Horizontal'
    void HandleCameraHorizontal();
    /// Handle 'Scene.Camera.Quad'
    void HandleCameraQuad();
    /// Handle 'Scene.Camera.Top1_Bottom2'
    void HandleCameraTop1Bottom2();
    /// Handle 'Scene.Camera.Top2_Bottom1'
    void HandleCameraTop2Bottom1();
    /// Handle 'Scene.Camera.Left1_Right2'
    void HandleCameraLeft1Right2();
    /// Handle 'Scene.Camera.Left2_Right1'
    void HandleCameraLeft2Right1();

    /// Handle viewports changed.
    void HandleViewportsChanged();

    /// Handle key press.
    void HandleKeyPress(QKeyEvent* event);
    /// Handle key release.
    void HandleKeyRelease(QKeyEvent* event);
    /// Handle mouse wheel.
    void HandleMouseWheel(QWheelEvent* event);
    /// Handle focus out.
    void HandleFocusOut();

private:
    /// Handle update.
    virtual void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle mouse button up/down.
    virtual void HandleMouseButton(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle post-render update.
    virtual void HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle node removed.
    void HandleNodeRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle component removed.
    void HandleComponentRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

private:
    /// Handle current document changed.
    virtual void HandleCurrentDocumentChanged(Document* document) override;
    /// Load the document from file.
    virtual bool DoLoad(const QString& fileName) override;

private:
    /// Gather nodes and components selection.
    void GatherSelection();
    /// Check for existing global component.
    bool CheckForExistingGlobalComponent(Urho3D::Node& node, const Urho3D::String& typeName);

private:
    /// Input subsystem.
    Urho3D::Input& input_;
    /// Widget.
    Urho3DWidget& widget_;
    /// Mouse buttons are down.
    QSet<Qt::MouseButton> mouseButtonsDown_;
    /// Mouse buttons are pressed.
    QSet<Qt::MouseButton> mouseButtonsPressed_;
    /// Consumed mouse buttons.
    QSet<Qt::MouseButton> mouseButtonsConsumed_;
    /// Keys are down.
    QSet<Qt::Key> keysDown_;
    /// Keys are pressed.
    QSet<Qt::Key> keysPressed_;
    /// Wheel delta.
    int wheelDelta_;
    /// Shows whether mouse move is consumed.
    bool mouseMoveConsumed_;

    /// Overlays.
    QList<SceneOverlay*> overlays_;

    /// Scene.
    Urho3D::SharedPtr<Urho3D::Scene> scene_;
    /// Viewport manager.
    QScopedPointer<SceneViewportManager> viewportManager_;

    /// Undo stack.
    QUndoStack undoStack_;
    /// Copy buffer.
    QVector<Urho3D::SharedPtr<Urho3D::XMLFile>> copyBuffer_;

    /// Selected objects.
    QSet<Urho3D::Object*> selectedObjects_;
    /// Selected nodes.
    NodeSet selectedNodes_;
    /// Selected components.
    ComponentSet selectedComponents_;
    /// Selected nodes and components.
    NodeSet selectedNodesAndComponents_;
    /// Last center of selected nodes and components.
    Urho3D::Vector3 lastSelectedCenter_;

};

}

