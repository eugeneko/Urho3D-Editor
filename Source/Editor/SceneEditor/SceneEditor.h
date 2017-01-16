#pragma once

#include "../Module.h"
#include "../Action.h"
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

