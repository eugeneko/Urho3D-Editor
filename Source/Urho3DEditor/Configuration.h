#pragma once

#include "Module.h"
#include <QSettings>
#include <QVector>

namespace Urho3DEditor
{

class Configuration : public QObject
{
    Q_OBJECT

public:
    /// Variable description.
    struct VariableDesc
    {
        /// Name.
        QString name_;
        /// Default value.
        QVariant defaultValue_;
        /// Display text.
        QString displayText_;
        /// Decoration info.
        QVariant decoration_;
    };
    /// Name of default section
    static const QString DefaultSection;
    /// Map of variables.
    using VariableMap = QHash<QString, QVariant>;
    /// Map of sections.
    using SectionMap = QHash<QString, QVector<VariableDesc>>;

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
        const QString& section = QString(), const QString& displayText = QString(), const QVariant& decoration = QVariant());

    /// Get default value of variable.
    QVariant GetDefaultValue(const QString& key);
    /// Get value of variable.
    QVariant GetValue(const QString& key);
    /// Set value of variable.
    void SetValue(const QString& key, const QVariant& value, bool saveImmediately = false);
    /// Get all registered variables per-section.
    const SectionMap& GetSections() const { return sections_; }

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
    VariableMap variables_;
    /// Default values.
    VariableMap defaultValues_;
    /// Sections.
    QHash<QString, QVector<VariableDesc>> sections_;

};

}

