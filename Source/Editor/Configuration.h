#pragma once

#include "Module.h"
#include <QSettings>

namespace Urho3DEditor
{

class ConfigurationVariable
{
public:
    /// Construct.
    ConfigurationVariable(const QString& variable, const QVariant& defaultValue, const QString& description);

private:
};

class Configuration : public Module
{
    Q_OBJECT

public:
    static const QString CORE_LASTDIRECTORY;
    static const QString PROJECT_RECENT;

public:
    /// Construct.
    Configuration();
    /// Destruct.
    virtual ~Configuration();
    /// Save setting.
    virtual void Save();
    /// Set default value for variable.
    virtual void SetDefault(const QString& key, const QVariant& value);
    /// Get value of variable.
    virtual QVariant GetValue(const QString& key);
    /// Set value of variable.
    virtual void SetValue(const QString& key, const QVariant& value, bool saveImmediately = false);

    // #TODO Hide me
    /// Get last directory.
    QString GetLastDirectory() const { return settings_.value(CORE_LASTDIRECTORY).toString(); }
    /// Set last directory.
    void SetLastDirectory(const QString& value) { settings_.setValue(CORE_LASTDIRECTORY, value); }
    /// Set last directory by file name.
    void SetLastDirectoryByFileName(const QString& fileName);

    /// Get recent projects.
    QStringList GetRecentProjects() const { return settings_.value(PROJECT_RECENT).toStringList(); }
    /// Add recent project.
    void AddRecentProject(const QString& name);

private:
    /// Settings.
    QSettings settings_;
    /// Stored variables.
    QHash<QString, QVariant> variables_;
    /// Default values.
    QHash<QString, QVariant> defaultValues_;

};

}

