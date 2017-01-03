#pragma once

#include "Module.h"
#include "MainWindow.h"
#include <QAction>
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

/// Scene Editor module.
class SceneEditor : public Module
{
    Q_OBJECT

public:
    /// Construct.
    SceneEditor();

protected:
    /// Initialize module.
    virtual bool DoInitialize() override;

protected slots:
    /// Handle 'File/New Scene'
    virtual void HandleFileNewScene();
    /// Handle 'File/Open Scene'
    virtual void HandleFileOpenScene();
    /// Handle 'Create/Replicated Node'
    virtual void HandleCreateReplicatedNode();
    /// Handle 'Create/Local Node'
    virtual void HandleCreateLocalNode();
    /// Handle current page changed.
    virtual void HandleCurrentPageChanged(MainWindowPage* page);

private:
    /// Update menu visibility.
    virtual void UpdateMenuVisibility();

private:
    /// Main window.
    MainWindow* mainWindow_;
    /// 'File/New Scene' action.
    QScopedPointer<QAction> actionFileNewScene_;
    /// 'File/Open Scene' action.
    QScopedPointer<QAction> actionFileOpenScene_;
    /// 'Create' menu.
    QScopedPointer<QMenu> menuCreate_;
    /// 'Create/Replicated Node' action.
    QScopedPointer<QAction> actionCreateReplicatedNode_;
    /// 'Create/Local Node' action.
    QScopedPointer<QAction> actionCreateLocalNode_;

};

/// Scene camera.
class SceneCamera
{
public:
    /// Construct.
    SceneCamera(Urho3D::Context* context);
    /// Get camera.
    Urho3D::Camera& GetCamera() const { return *camera_; }

    /// Set grab mouse.
    void SetGrabMouse(bool grab);
    /// Move camera.
    void Move(const Urho3D::Vector3& movement, const Urho3D::Vector3& rotation);

private:
    /// Input subsystem.
    Urho3D::Input* input_;
    /// Camera node.
    Urho3D::Node cameraNode_;
    /// Camera component.
    Urho3D::Camera* camera_;
    /// Camera angles.
    Urho3D::Vector3 angles_;
};

/// Scene page.
class ScenePage : public MainWindowPage, public Urho3D::Object
{
    Q_OBJECT
    URHO3D_OBJECT(ScenePage, Urho3D::Object);

public:
    /// Construct.
    ScenePage(MainWindow& mainWindow);
    /// Get scene.
    Urho3D::Scene& GetScene() const { return *scene_; }

    /// Return title of the page.
    virtual QString GetTitle() override { return GetRawTitle(); }
    /// Return whether the page can be saved.
    virtual bool CanBeSaved() override { return true; }
    /// Return whether the page widget should be visible when the page is active.
    virtual bool IsPageWidgetVisible() override { return false; }
    /// Return whether the Urho3D widget should be visible when the page is active.
    virtual bool IsUrho3DWidgetVisible() override { return true; }
    /// Get name filters for open and save dialogs.
    virtual QString GetNameFilters() override;

private:
    /// Handle update.
    virtual void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle mouse button up/down.
    virtual void HandleMouseButton(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

protected:
    /// Handle current page changed.
    virtual void HandleCurrentPageChanged(MainWindowPage* page) override;
    /// Load the page from file.
    virtual bool DoLoad(const QString& fileName) override;

private:
    /// Widget.
    Urho3DWidget* widget_;
    /// Camera.
    SceneCamera camera_;
    /// Scene.
    Urho3D::SharedPtr<Urho3D::Scene> scene_;
    /// Viewport.
    Urho3D::SharedPtr<Urho3D::Viewport> viewport_;

    /// Selected nodes.

};

}

