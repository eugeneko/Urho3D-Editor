#pragma once

#include "EditorInterfaces.h"
#include <Urho3D/Scene/Node.h>

namespace Urho3D
{

class Transformable;

class StaticModel;
class Model;
class Material;

/// Gizmo axis.
struct GizmoAxis
{
    Ray axisRay;
    bool selected = false;
    bool lastSelected = false;
    float t = 0.0f;
    float d = 0.0f;
    float lastT = 0.0f;
    float lastD = 0.0f;

    void Update(Ray cameraRay, float scale, bool drag, float axisMaxT, float axisMaxD);

    void Moved();
};

/// Gizmo type.
enum class GizmoType
{
    Position,
    Rotation,
    Scale,
    Select,
    COUNT
};

/// Gizmo axis mode.
enum class GizmoAxisMode
{
    Local,
    World
};

/// Gizmo.
class Gizmo : public AbstractEditorOverlay
{
    URHO3D_OBJECT(Gizmo, AbstractEditorOverlay);

public:
    std::function<void()> onChanged_;

public:
    /// Construct.
    Gizmo(Context* context);
    /// Set transformable.
    void SetTransformable(Transformable* transformable) { transformable_ = transformable; }
    /// Set gizmo mode.
    void SetGizmoType(GizmoType type, float step = 1.0f, float snapScale = 0.0f);
    /// Set axis mode.
    void SetAxisMode(GizmoAxisMode axisMode) { axisMode_ = axisMode; }

    /// Return whether the gizmo is snapped.
    bool IsSnapped() const { return snapScale_ != 0.0f; }

private:
    /// @see AbstractEditorOverlay::Update
    void Update(AbstractInput& input, AbstractEditorContext& editorContext, float timeStep) override;

private:
    /// Get gizmo model.
    Model* GetGizmoModel(GizmoType type);
    /// Get gizmo material.
    Material* GetGizmoMaterial(int axis, bool highlight);

    /// Show gizmo.
    void ShowGizmo();
    /// Hide gizmo.
    void HideGizmo();

    /// Update drag state.
    void UpdateDragState(AbstractInput& input);
    /// Prepare undo action if dragging started.
    void PrepareUndo();
    /// Flush undo action if dragging started, update previous drag states.
    void FinalizeUndo();
    /// Check whether gizmo is dragging.
    bool IsDragging() const { return keyDrag_ || mouseDrag_; }
    /// Check whether gizmo was dragging.
    bool WasDragging() const { return lastKeyDrag_ || lastMouseDrag_; }

    /// Position gizmo.
    void PositionGizmo();
    /// Resize gizmo.
    void ResizeGizmo(AbstractEditorContext& editorContext);
    /// Calculate gizmo axes.
    void CalculateGizmoAxes();
    /// Mark gizmo moved.
    void MarkMoved();
    /// Flush gizmo actions.
    void FlushActions();
    /// Use gizmo (by keyboard).
    void UseGizmoKeyboard(AbstractInput& input, float timeStep);
    /// Use gizmo (by mouse). Return true if selected.
    bool UseGizmoMouse(const Ray& mouseRay);

    /// Move edited nodes.
    bool MoveNodes(Vector3 adjust);
    /// Rotate edited nodes.
    bool RotateNodes(Vector3 adjust);
    /// Scale edited nodes.
    bool ScaleNodes(Vector3 adjust);

    /// Signal that gizmo is changed
    void OnChanged();

private:
    /// Transformable.
    Transformable* transformable_ = nullptr;

    /// Gizmo node.
    Node gizmoNode_;
    /// Static model component.
    StaticModel& gizmo_;

    /// Gizmo type.
    GizmoType gizmoType_ = GizmoType::Position;
    /// Step.
    float step_ = 1.0f;
    /// Snap step.
    float snapScale_ = 0.0f;
    /// Axis mode.
    GizmoAxisMode axisMode_ = GizmoAxisMode::World;

    /// X axis of gizmo.
    GizmoAxis axisX_;
    /// Y axis of gizmo.
    GizmoAxis axisY_;
    /// Z axis of gizmo.
    GizmoAxis axisZ_;

    /// Is dragging by mouse?
    bool mouseDrag_ = false;
    /// Was dragging by mouse?
    bool lastMouseDrag_ = false;
    /// Is dragging by keyboard?
    bool keyDrag_ = false;
    /// Was dragging by keyboard?
    bool lastKeyDrag_ = false;
    /// Was moved?
    bool moved_ = false;

};

}
