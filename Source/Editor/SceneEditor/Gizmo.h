#pragma once

#include <Urho3D/Scene/Node.h>
#include <QObject>

namespace Urho3D
{

class StaticModel;

}

namespace Urho3DEditor
{

class ScenePage;

/// Gizmo manager.
class Gizmo : public QObject
{
    Q_OBJECT

public:
    /// Gizmo type.
    enum GizmoType
    {
        GizmoNone,
        GizmoPosition,
        GizmoRotation,
        GizmoScale
    };

public:
    /// Construct.
    Gizmo(ScenePage& page);
    /// Destruct.
    virtual ~Gizmo();
    /// Set gizmo type.
    void SetType(GizmoType type);

protected slots:
    /// Handle selection changed.
    virtual void HandleSelectionChanged();

private:
    /// Hide gizmo.
    void HideGizmo();
    /// Create gizmo and place it to scene.
    void CreateGizmo();

private:
    /// Page.
    ScenePage& page_;
    /// Gizmo node.
    Urho3D::Node gizmoNode_;
    /// Static model.
    Urho3D::StaticModel& gizmo_;
    /// Current gizmo type.
    GizmoType type_;

};

}
