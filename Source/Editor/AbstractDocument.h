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

namespace Urho3D
{

class Urho3DWidget;

class AbstractPage : public QWidget, public Object
{
    Q_OBJECT
    URHO3D_OBJECT(AbstractPage, Object);

public:
    /// Construct.
    AbstractPage(Context* context);
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
    StartPage(Context* context);

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
    Context* GetContext() const;

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
    void SetScene(SharedPtr<Scene> scene);

private:
    /// Setup viewport.
    void SetupViewport();

    /// Handle update.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle key up/down.
    void HandleKey(StringHash eventType, VariantMap& eventData);
    /// Handle mouse button up/down.
    void HandleMouseButton(StringHash eventType, VariantMap& eventData);

private:
    /// Camera node.
    Node cameraNode_;
    /// Camera component.
    Camera* camera_;

    /// Scene.
    SharedPtr<Scene> scene_;
    /// Viewport.
    SharedPtr<Viewport> viewport_;
};

}

