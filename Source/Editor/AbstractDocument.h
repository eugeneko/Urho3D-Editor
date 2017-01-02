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

}

