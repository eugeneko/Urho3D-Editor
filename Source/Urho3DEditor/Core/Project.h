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

/// Urho3D Project description.
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

    /// Get absolute resource prefix paths.
    QString GetAbsoluteResourcePrefixPaths() const;
    /// Get resource cache parameters.
    Urho3D::VariantMap GetResourceCacheParameters() const;
    /// Get resource prefix paths.
    const QString& GetResourcePrefixPaths() const { return resourcePrefixPaths_; }
    /// Get resource paths.
    const QString& GetResourcePaths() const { return resourcePaths_; }
    /// Get auto-load paths.
    const QString& GetAutoloadPaths() const { return autoloadPaths_; }
    /// Get package paths.
    const QString& GetPackagePaths() const { return packagePaths_; }
    /// Get default render path.
    const QString& GetDefaultRenderPath() const { return defaultRenderPath_; }

    /// Set resource prefix paths.
    void SetResourcePrefixPaths(const QString& prefixPaths) { resourcePrefixPaths_ = prefixPaths; }
    /// Set resource paths.
    void SetResourcePaths(const QString& paths) { resourcePaths_ = paths; }
    /// Set auto-load paths.
    void SetAutoloadPaths(const QString& autoloadPaths) { autoloadPaths_ = autoloadPaths; }
    /// Set package paths.
    void SetPackagePaths(const QString& packagePaths) { packagePaths_ = packagePaths; }
    /// Set default render path.
    void SetDefaultRenderPath(const QString& defaultRenderPath) { defaultRenderPath_ = defaultRenderPath; }

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

/// Target type.
enum class TargetType
{

};

/// Run-able target description.
struct Target
{

};

}

