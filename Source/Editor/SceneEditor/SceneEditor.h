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
    static const QString VarHotkeyMode;

public:
    /// Construct.
    SceneEditor();

protected:
    /// Initialize module.
    virtual bool Initialize() override;

protected slots:
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

