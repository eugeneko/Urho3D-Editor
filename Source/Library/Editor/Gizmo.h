#pragma once

#include "EditorInterfaces.h"
#include "../AbstractUI/KeyBinding.h"
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
    /// Gizmo controls.
    enum Control
    {
        DRAG_GIZMO,
        SNAP_DRAG,
        STEPPED_X_POS,
        STEPPED_X_NEG,
        STEPPED_Y_POS,
        STEPPED_Y_NEG,
        STEPPED_Z_POS,
        STEPPED_Z_NEG,
        STEPPED_UPSCALE,
        STEPPED_DOWNSCALE,
        SMOOTH_X_POS,
        SMOOTH_X_NEG,
        SMOOTH_Y_POS,
        SMOOTH_Y_NEG,
        SMOOTH_Z_POS,
        SMOOTH_Z_NEG,
        SMOOTH_UPSCALE,
        SMOOTH_DOWNSCALE,
    };
    using Controls = HashMap<int, CompositeKeyBinding>;

public:
    /// Construct.
    Gizmo(Context* context);
    /// Set controls.
    void SetControls(const Controls& controls) { controls_ = controls; }
    /// Set transformable.
    void SetTransformable(Transformable* transformable) { transformable_ = transformable; }
    /// Set gizmo mode.
    void SetGizmoType(GizmoType type, float step = 1.0f);
    /// Set axis mode.
    void SetAxisMode(GizmoAxisMode axisMode) { axisMode_ = axisMode; }

    /// Return whether the gizmo is snapped.
    bool IsSnapped() const { return step_ != 0.0f; }

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

    /// Position gizmo.
    void PositionGizmo();
    /// Resize gizmo.
    void ResizeGizmo(AbstractEditorContext& editorContext);
    /// Calculate gizmo axes.
    void CalculateGizmoAxes();
    /// Mark gizmo moved.
    void MarkMoved();
    /// Start transformation.
    void EnsureTransformationStarted();
    /// Use gizmo (by keyboard).
    void UseGizmoKeyboard(AbstractInput& input, float timeStep);
    /// Use gizmo (by mouse). Return true if selected.
    void UseGizmoMouse(AbstractInput& input, const Ray& mouseRay);

    /// Move edited nodes.
    bool MoveNodes(Vector3 adjust, bool snap);
    /// Rotate edited nodes.
    bool RotateNodes(Vector3 adjust, bool snap);
    /// Scale edited nodes.
    bool ScaleNodes(Vector3 adjust, bool snap);

    /// Signal that gizmo is changed
    void OnChanged();

private:
    /// Controls.
    Controls controls_;
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
    /// Axis mode.
    GizmoAxisMode axisMode_ = GizmoAxisMode::World;

    /// X axis of gizmo.
    GizmoAxis axisX_;
    /// Y axis of gizmo.
    GizmoAxis axisY_;
    /// Z axis of gizmo.
    GizmoAxis axisZ_;

    /// Whether the gizmo is transforming now.
    bool transforming_ = false;
    /// Whether ths gizmo is dragged by mouse.
    bool dragging_ = false;

};

}
