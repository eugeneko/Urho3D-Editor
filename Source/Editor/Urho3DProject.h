#pragma once

#include "Module.h"
#include "AbstractDocument.h"
#include <Urho3D/Core/Context.h>
// #include <Urho3D/Engine/Engine.h>
// #include <Urho3D/Graphics/Viewport.h>
// #include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/Resource.h>
// #include <QApplication>
// #include <QMainWindow>
// #include <QMenuBar>
#include <QString>
#include <QStringList>

class QFormLayout;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

namespace Urho3DEditor
{

class MainWindow;

/// Project support.
class ProjectManager : public Module
{
    Q_OBJECT

public:
    /// Construct.
    ProjectManager();

protected:
    /// Initialize module.
    virtual bool DoInitialize() override;

protected slots:
    /// Handle 'File/New Project'
    virtual void HandleFileNewProject();

private:
    /// Main window.
    MainWindow* mainWindow_;
    /// 'File/New Project' action.
    QScopedPointer<QAction> actionFileNewProject_;
};

/// Urho3D Project document.
class Urho3DProject : public Urho3D::Resource
{
    URHO3D_OBJECT(Urho3DProject, Urho3D::Resource);

public:
    /// Construct.
    Urho3DProject(Urho3D::Context* context);
    /// Concatenate path list to string.
    static QString ConcatenateList(const QStringList& list, QChar separator = ';');
    /// Get base path of the project.
    QString GetBasePath() const;

    /// Set resource prefix paths.
    void SetResourcePrefixPaths(const QString& prefixPaths);
    /// Set resource paths.
    void SetResourcePaths(const QString& paths);

    /// Get resource prefix paths.
    const QString& GetResourcePrefixPaths() const { return resourcePrefixPaths_; }
    /// Get absolute resource prefix paths list.
    QStringList GetAbsoluteResourcePrefixPathsList(const QString& basePath) const;
    /// Get absolute resource prefix paths.
    QString GetAbsoluteResourcePrefixPaths(const QString& basePath) const;
    /// Get resource paths.
    const QString& GetResourcePaths() const { return resourcePaths_; }

    /// Get resource prefix paths (as is).
    QStringList GetResourcePrefixPathsList() const;
    /// Get resource paths.
    QStringList GetResourcePathsList() const;
    /// Get available resource paths.
    QStringList GetAvailableResourcePathsList(const QString& basePath) const;

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    virtual bool BeginLoad(Urho3D::Deserializer& source) override;
    /// Save resource. Return true if successful.
    virtual bool Save(Urho3D::Serializer& dest) const override;

private:
    /// Resource Prefix Paths.
    QString resourcePrefixPaths_;
    /// Resource Paths.
    QString resourcePaths_;

};

/// Urho3D Project properties page.
class Urho3DProjectPage : public AbstractPage
{
    Q_OBJECT
    URHO3D_OBJECT(Urho3DProjectPage, AbstractPage);

public:
    /// Construct.
    Urho3DProjectPage(Urho3D::Context* context);
    /// Get project.
    Urho3D::SharedPtr<Urho3DProject> GetProject() { return project_; }

    /// Return default file name for save dialog.
    virtual QString GetDefaultFileName() const { return "Project.urho"; }
    /// Return filters.
    virtual QString GetFilters() const override;

protected:
    /// Save page.
    virtual bool DoSave() override;
    /// Load page.
    virtual bool DoLoad() override;

private slots:
    /// Handle 'Resource Prefix Paths' edited.
    void OnResourcePrefixPathsEdited(const QString& value);
    /// Handle 'Resource Paths' edited.
    void OnResourcePathsEdited(const QString& value);

private:
    /// Project.
    Urho3D::SharedPtr<Urho3DProject> project_;
    /// Layout.
    QGridLayout* layout_;
    /// 'Set as Current' button.
    QPushButton* buttonSetAsCurrent_;
    /// 'Resource Prefix Paths' field.
    QLineEdit* fieldResourcePrefixPaths_;
    /// 'Resource Paths' field.
    QLineEdit* fieldResourcePaths_;

};

}

