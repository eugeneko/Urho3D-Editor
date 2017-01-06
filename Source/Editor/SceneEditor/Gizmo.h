#pragma once

#include "../Module.h"
#include <Urho3D/Scene/Node.h>
#include <QObject>

namespace Urho3D
{

class StaticModel;

}

namespace Urho3DEditor
{

class Document;
class SceneDocument;

/// Gizmo manager.
class GizmoManager : public Module
{
    Q_OBJECT

public:
    /// 'Gizmo Type' variable.
    static const QString VarGizmoMode;

public:

private:
    /// Initialize module.
    virtual bool Initialize() override;

private slots:
    /// Handle current document changed.
    virtual void HandleCurrentPageChanged(Document* document);

};

/// Gizmo.
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
    Gizmo(SceneDocument& document);
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
    /// Document.
    SceneDocument& document_;
    /// Gizmo node.
    Urho3D::Node gizmoNode_;
    /// Static model.
    Urho3D::StaticModel& gizmo_;
    /// Current gizmo type.
    GizmoType type_;

};

}
