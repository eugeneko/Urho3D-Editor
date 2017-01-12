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

/// Gizmo manager.
class GizmoManager : public Module
{
    Q_OBJECT

public:
    /// Controls type of gizmo.
    static const QString VarGizmoType;
    /// Controls axis mode.
    static const QString VarGizmoAxisMode;

    /// Controls snap step multiplier.
    static const QString VarSnapFactor;
    /// Controls whether the position change is snapped to step.
    static const QString VarSnapPosition;
    /// Controls whether the rotation change is snapped to step.
    static const QString VarSnapRotation;
    /// Controls whether the scale change is snapped to step.
    static const QString VarSnapScale;
    /// Controls the step of position snapping.
    static const QString VarSnapPositionStep;
    /// Controls the step of rotation snapping.
    static const QString VarSnapRotationStep;
    /// Controls the step of scale snapping.
    static const QString VarSnapScaleStep;

    /// Controls 3D model of gizmo axes (edit position).
    static const QString VarModelPosition;
    /// Controls 3D model of gizmo axes (edit rotation).
    static const QString VarModelRotation;
    /// Controls 3D model of gizmo axes (edit scale).
    static const QString VarModelScale;

    /// Controls material of X axis.
    static const QString VarMaterialRed;
    /// Controls material of Y axis.
    static const QString VarMaterialGreen;
    /// Controls material of Z axis.
    static const QString VarMaterialBlue;

    /// Controls material of X axis (highlighted)
    static const QString VarMaterialRedHighlight;
    /// Controls material of Y axis (highlighted)
    static const QString VarMaterialGreenHighlight;
    /// Controls material of Z axis (highlighted)
    static const QString VarMaterialBlueHighlight;

public:

private:
    /// Initialize module.
    virtual bool Initialize() override;

private slots:
    /// Handle current document changed.
    virtual void HandleCurrentPageChanged(Document* document);

};

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
    virtual void Update(SceneInputInterface& input, const Urho3D::Ray& cameraRay, float timeStep) override;

private:
    /// Get gizmo model.
    Urho3D::Model* GetGizmoModel(GizmoType type);
    /// Get gizmo material.
    Urho3D::Material* GetGizmoMaterial(int axis, bool highlight);

    /// Show gizmo.
    void ShowGizmo();
    /// Hide gizmo.
    void HideGizmo();

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
    /// Use gizmo.
    void UseGizmo(const Urho3D::Ray& cameraRay);

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
    GizmoType lastType_;

    /// X axis of gizmo.
    GizmoAxis axisX_;
    /// Y axis of gizmo.
    GizmoAxis axisY_;
    /// Z axis of gizmo.
    GizmoAxis axisZ_;

    /// Is dragging?
    bool drag_;
    /// Was dragging?
    bool lastDrag_;
    /// Edited nodes.
    QList<Urho3D::Node*> editNodes_;
    /// Old transforms.
    QVector<NodeTransform> oldTransforms_;
    /// Was moved?
    bool moved_;

};

}
