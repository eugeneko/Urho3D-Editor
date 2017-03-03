#pragma once

#include "../Module.h"
#include "../Core/Document.h"
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

/// Project.
class Project : public Urho3D::Resource
{
    URHO3D_OBJECT(Project, Urho3D::Resource);

public:
    /// Construct.
    Project(Urho3D::Context* context);
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

}

