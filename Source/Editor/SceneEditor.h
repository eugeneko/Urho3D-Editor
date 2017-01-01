#pragma once

#include "Module.h"
#include "MainWindow.h"
#include <QAction>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>

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

private:
    /// Main window.
    MainWindow* mainWindow_;
    /// 'File/New Scene' action.
    QScopedPointer<QAction> actionFileNewScene_;
    /// 'File/Open Scene' action.
    QScopedPointer<QAction> actionFileOpenScene_;

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
    virtual QString GetTitle() { return GetRawTitle(); }
    /// Return whether the page can be saved.
    virtual bool CanBeSaved() { return true; }
    /// Return whether the page widget should be visible when the page is active.
    virtual bool IsPageWidgetVisible() { return false; }
    /// Return whether the Urho3D widget should be visible when the page is active.
    virtual bool IsUrho3DWidgetVisible() { return true; }

protected:
    /// Handle current page changed.
    virtual void HandleCurrentPageChanged(MainWindowPage* page) override;
    /// Load the page from file.
    virtual bool DoLoad(const QString& fileName) override;

private:
    /// Camera node.
    Urho3D::Node cameraNode_;
    /// Camera component.
    Urho3D::Camera* camera_;

    /// Scene.
    Urho3D::SharedPtr<Urho3D::Scene> scene_;
    /// Viewport.
    Urho3D::SharedPtr<Urho3D::Viewport> viewport_;

};

}

