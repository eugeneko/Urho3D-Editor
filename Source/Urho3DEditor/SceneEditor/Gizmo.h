#pragma once

#include "SceneOverlay.h"
#include "../Module.h"
#include <Urho3D/Scene/Node.h>
#include <QList>
#include <QObject>
#include <QSet>
#include <QVector>

namespace Urho3D
{

class StaticModel;
class Model;
class Material;

}

namespace Urho3DEditor
{

class Document;
class SceneDocument;
struct NodeTransform;

/// Gizmo axis.
struct GizmoAxis
{
    Urho3D::Ray axisRay;
    bool selected;
    bool lastSelected;
    float t;
    float d;
    float lastT;
    float lastD;

    GizmoAxis();

    void Update(Urho3D::Ray cameraRay, float scale, bool drag, float axisMaxT, float axisMaxD);

    void Moved();
};

/// Gizmo type.
enum class GizmoType
{
    Position,
    Rotation,
    Scale,
    Select
};

/// Gizmo axis mode.
enum class GizmoAxisMode
{
    Local,
    World
};

/// Gizmo.
class Gizmo : public QObject, public SceneOverlay
{
    Q_OBJECT

public:
    /// Construct.
    Gizmo(SceneDocument& document);
    /// Destruct.
    virtual ~Gizmo();

private:
    /// @see SceneOverlay::Update
    virtual void Update(SceneInputInterface& input, float timeStep) override;

private:
    /// Get gizmo model.
    Urho3D::Model* GetGizmoModel(GizmoType type);
    /// Get gizmo material.
    Urho3D::Material* GetGizmoMaterial(int axis, bool highlight);

    /// Show gizmo.
    void ShowGizmo();
    /// Hide gizmo.
    void HideGizmo();

    /// Update drag state.
    void UpdateDragState(SceneInputInterface& input);
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
    void ResizeGizmo();
    /// Calculate gizmo axes.
    void CalculateGizmoAxes();
    /// Mark gizmo moved.
    void MarkMoved();
    /// Flush gizmo actions.
    void FlushActions();
    /// Use gizmo (by keyboard).
    void UseGizmoKeyboard(SceneInputInterface& input, float timeStep);
    /// Use gizmo (by mouse). Return true if selected.
    bool UseGizmoMouse(const Urho3D::Ray& mouseRay);

    /// Move edited nodes.
    bool MoveNodes(Urho3D::Vector3 adjust);
    /// Rotate edited nodes.
    bool RotateNodes(Urho3D::Vector3 adjust);
    /// Scale edited nodes.
    bool ScaleNodes(Urho3D::Vector3 adjust);

private:
    /// Document.
    SceneDocument& document_;
    /// Gizmo node.
    Urho3D::Node gizmoNode_;
    /// Static model component.
    Urho3D::StaticModel& gizmo_;

    /// Previous type of gizmo.
    GizmoType lastType_ = GizmoType::Position;

    /// X axis of gizmo.
    GizmoAxis axisX_;
    /// Y axis of gizmo.
    GizmoAxis axisY_;
    /// Z axis of gizmo.
    GizmoAxis axisZ_;

    /// Is dragging by mouse?
    bool mouseDrag_;
    /// Was dragging by mouse?
    bool lastMouseDrag_;
    /// Is dragging by keyboard?
    bool keyDrag_;
    /// Was dragging by keyboard?
    bool lastKeyDrag_;
    /// Edited nodes.
    QList<Urho3D::Node*> editNodes_;
    /// Old transforms.
    QVector<NodeTransform> oldTransforms_;
    /// Was moved?
    bool moved_;

};

}
