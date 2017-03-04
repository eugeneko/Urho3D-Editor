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

/// Project.
class Project : public QObject
{
    Q_OBJECT

public:
    /// Construct.
    Project();
    /// Set project file name.
    void SetFileName(const QString& fileName) { fileName_ = fileName; }
    /// Get project file name.
    QString GetFileName() const { return fileName_; }
    /// Get project short title name.
    QString GetTitle() const;
    /// Save project.
    bool Save();
    /// Load project.
    bool Load();

    /// Get resource cache parameters.
    Urho3D::VariantMap GetResourceCacheParameters() const;
    /// Get default render path.
    QString GetDefaultRenderPath() const { return defaultRenderPath_; }

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

private:
    /// Project file name.
    QString fileName_;

    /// Resource prefix paths.
    QString resourcePrefixPaths_;
    /// Resource paths.
    QString resourcePaths_;
    /// Auto-load paths.
    QString autoloadPaths_;
    /// Package paths.
    QString packagePaths_;
    /// Default render path.
    QString defaultRenderPath_;

};

}

