#pragma once

#include "../Module.h"
#include "../Core/Document.h"
#include <QPushButton>
#include <QGridLayout>
#include <QLineEdit>
#include <QString>
#include <QStringList>

namespace Urho3DEditor
{

class Core;
class Project;

/// Document that shows project content.
class ProjectDocument : public Document
{
    Q_OBJECT

public:
    /// Construct.
    ProjectDocument(Core& core);
    /// Get project.
    QSharedPointer<Project> GetProject() { return project_; }

private:
    /// Load the document from file.
    virtual bool DoLoad(const QString& fileName);
    /// Save the document to file.
    virtual bool DoSave(const QString& fileName);

private:
    /// Project.
    QSharedPointer<Project> project_;
    /// Layout.
    QGridLayout* layout_;
    /// 'Set as Current' button.
    QPushButton* buttonSetAsCurrent_;
    /// 'Resource Prefix Paths' field.
    QLineEdit* fieldResourcePrefixPaths_;
    /// 'Resource Paths' field.
    QLineEdit* fieldResourcePaths_;

};

/// ProjectDocument factory.
class ProjectDocumentFactory : public DocumentFactoryT<ProjectDocument>
{
public:
    /// @see DocumentFactory::IsSaveable
    virtual bool IsSaveable() const override { return true; }
    /// @see DocumentFactory::ShallSaveOnCreate
    virtual bool ShallSaveOnCreate() const override { return true; }
    /// @see DocumentFactory::GetDefaultFileName
    virtual QString GetDefaultFileName() const override { return "Project.urho"; }
    /// @see DocumentFactory::GetFileNameFilters
    virtual QStringList GetFileNameFilters() const override { return{ "Urho3D Project (*.urho)" }; }

};

}

