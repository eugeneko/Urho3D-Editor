#pragma once

#include "../Module.h"
#include <QAction>

namespace Urho3D
{

class Input;

}

namespace Urho3DEditor
{

class Document;
class MainWindow;
class SceneOverlay;
class Urho3DWidget;

/// Scene Editor module.
class SceneEditor : public Module
{
    Q_OBJECT

public:
    /// Controls hot key mode.
    static const QString VarHotKeyMode;
    /// Controls base camera speed.
    static const QString VarCameraBaseSpeed;
    /// Controls camera speed multiplication when Shift is pressed/
    static const QString VarCameraShiftSpeedMultiplier;
    /// Controls base camera rotation speed.
    static const QString VarCameraBaseRotationSpeed;
    /// Controls whether the mouse wheel control position instead of scrolling.
    static const QString VarMouseWheelCameraPosition;
    /// Controls whether the mouse middle button enables pan camera mode.
    static const QString VarMmbPanMode;
    /// Controls whether the camera pitch is limited by 90 degrees.
    static const QString VarLimitRotation;

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

    /// Controls type of objects that are picked by mouse.
    static const QString VarPickMode;

public:
    /// Construct.
    SceneEditor();

protected:
    /// Initialize module.
    virtual bool Initialize() override;

private slots:
    /// Handle 'File/New Scene'
    virtual void HandleFileNewScene();
    /// Handle 'File/Open Scene'
    virtual void HandleFileOpenScene();
    /// Handle 'Create/Replicated Node'
    virtual void HandleCreateReplicatedNode();
    /// Handle 'Create/Local Node'
    virtual void HandleCreateLocalNode();
    /// Handle current document changed.
    virtual void HandleCurrentPageChanged(Document* document);

private:
    /// Update menu visibility.
    virtual void UpdateMenuVisibility();

private:
    /// 'File/New Scene' action.
    QScopedPointer<QAction> actionFileNewScene_;
    /// 'File/Open Scene' action.
    QScopedPointer<QAction> actionFileOpenScene_;
    /// 'Create/Replicated Node' action.
    QScopedPointer<QAction> actionCreateReplicatedNode_;
    /// 'Create/Local Node' action.
    QScopedPointer<QAction> actionCreateLocalNode_;

};

}

