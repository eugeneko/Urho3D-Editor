#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Scene/Node.h>
#include <QApplication>
#include <QGridLayout>
#include <QMainWindow>
#include <QMenuBar>
#include <QPushButton>

class QFormLayout;
class QLineEdit;
class QListWidget;

namespace Urho3DEditor
{

class Urho3DWidget;

class AbstractPage : public QWidget, public Urho3D::Object
{
    Q_OBJECT
    URHO3D_OBJECT(AbstractPage, Urho3D::Object);

public:
    /// Construct.
    AbstractPage(Urho3D::Context* context);
    /// Destruct.
    virtual ~AbstractPage() {}

    /// Launch save dialog and get the destination file name.
    virtual bool LaunchSaveDialog();
    /// Save page.
    virtual bool Save(bool askForDestination);
    /// Launch open dialog and get the source file name.
    virtual bool LaunchOpenDialog();
    /// Open page.
    virtual bool Open(bool askForSource);

    /// Mark page as unsaved.
    void MarkUnsaved();
    /// Reset page unsaved flag.
    void ResetUnsaved();
    /// Set title of the page.
    void SetTitle(const QString& value);
    /// Get title of the page.
    QString GetTitle() const { return title_; }
    /// Get file name.
    QString GetFileName() const { return fileName_; }
    /// Return whether the Urho3D page has unsaved changes.
    bool HasUnsavedChanges() const { return unsavedChanges_; }

    /// Get title of the page (decorated).
    virtual QString GetTitleDecorated() const { return GetTitle() + (HasUnsavedChanges() ? " *" : ""); }
    /// Return whether the Urho3D widget shall be visible.
    virtual bool IsUrho3DWidgetVisible() const { return false; }
    /// Return default file name for save dialog.
    virtual QString GetDefaultFileName() const { return ""; }
    /// Return filters for save dialog.
    virtual QString GetFilters() const { return "All files (*.*)"; }

protected:
    /// Save page.
    virtual bool DoSave() { return true; }
    /// Load page.
    virtual bool DoLoad() { return true; }

signals:
    /// Signals that title of the page has changed.
    void titleChanged(AbstractPage* page, const QString& newTitle);

private:
    /// Title.
    QString title_;
    /// Destination file name.
    QString fileName_;
    /// Whether the page has unsaved changes.
    bool unsavedChanges_;

};

class StartPage : public AbstractPage
{
    Q_OBJECT
    URHO3D_OBJECT(StartPage, AbstractPage);

public:
    /// Construct.
    StartPage(Urho3D::Context* context);

private:
    /// Layout.
    QGridLayout* layout_;
    /// 'New Project' button.
    QPushButton* newProjectButton_;
    /// 'Recent Projects' list.
    QListWidget* recentProjects_;

};

class Urho3DPage : public AbstractPage
{
    Q_OBJECT
    URHO3D_OBJECT(Urho3DPage, AbstractPage);

public:
    /// Construct.
    Urho3DPage(Urho3DWidget* urho3dWidget, const QString& name);
    /// Destruct.
    virtual ~Urho3DPage();
    /// Get Urho3D widget.
    Urho3DWidget* GetUrho3DWidget() const { return urho3dWidget_; }
    /// Get Urho3D context.
    Urho3D::Context* GetContext() const;

    /// Return whether the Urho3D widget shall be visible.
    virtual bool IsUrho3DWidgetVisible() const { return true; }

private:
    /// Urho3D widget.
    Urho3DWidget* urho3dWidget_;

};

class SceneEditorPage : public Urho3DPage
{
    Q_OBJECT

public:
    /// Construct.
    SceneEditorPage(Urho3DWidget* urho3dWidget, const QString& name);
    /// Set scene.
    void SetScene(Urho3D::SharedPtr<Urho3D::Scene> scene);

private:
    /// Setup viewport.
    void SetupViewport();

    /// Handle update.
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle key up/down.
    void HandleKey(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle mouse button up/down.
    void HandleMouseButton(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

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

