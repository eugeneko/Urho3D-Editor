#pragma once

#include "Module.h"
#include "Document.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/Resource.h>
#include <QPushButton>
#include <QGridLayout>
#include <QLineEdit>
#include <QString>
#include <QStringList>

namespace Urho3DEditor
{

class Core;

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

/// Document that shows project content.
class ProjectDocument : public Document
{
    Q_OBJECT
    URHO3DEDITOR_DOCUMENT

    /// Create description.
    static DocumentDescription CreateDescription()
    {
        DocumentDescriptionT<ProjectDocument> desc;
        desc.saveOnCreate_ = true;
        desc.fileNameFilters_ << "Urho3D Project (*.urho)";
        desc.defaultFileName_ = "Project.urho";
        return desc;
    }

public:
    /// Construct.
    ProjectDocument(Core& core);
    /// Get project.
    QSharedPointer<Urho3DProject> GetProject() { return project_; }

private:
    /// Load the document from file.
    virtual bool DoLoad(const QString& fileName);
    /// Save the document to file.
    virtual bool DoSave(const QString& fileName);

private:
    /// Project.
    QSharedPointer<Urho3DProject> project_;
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

