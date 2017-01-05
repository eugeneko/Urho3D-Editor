#pragma once

#include "Module.h"
#include <QSettings>

namespace Urho3DEditor
{

class Configuration : public QObject
{
    Q_OBJECT

public:
    static const QString CORE_LASTDIRECTORY;
    static const QString PROJECT_RECENT;

public:
    /// Construct.
    Configuration();
    /// Destruct.
    ~Configuration();
    /// Save setting.
    void Save();

    /// Register variable.
    void RegisterVariable(const QString& key, const QVariant& defaultValue,
        const QString& comment = QString(), const QVariant& decoration = QVariant());
    /// Get value of variable.
    QVariant GetValue(const QString& key);
    /// Set value of variable.
    void SetValue(const QString& key, const QVariant& value, bool saveImmediately = false);
    /// Get comment.
    QString GetComment(const QString& key) const;
    /// Get decoration info.
    QVariant GetDecoration(const QString& key) const;
    /// Get all registered variables with default values.
    const QHash<QString, QVariant>& GetVariables() const { return defaultValues_; }

    // #TODO Remove me
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
    /// Comments.
    QHash<QString, QString> comments_;
    /// Decorations.
    QHash<QString, QVariant> decorations_;

};

}

