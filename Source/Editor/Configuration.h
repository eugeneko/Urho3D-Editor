#pragma once

#include "Module.h"
#include <QSettings>

namespace Urho3DEditor
{

class Configuration : public Module
{
    Q_OBJECT

public:
    static const QString CORE_LASTDIRECTORY;
    static const QString PROJECT_RECENT;

public:
    /// Construct.
    Configuration();

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

};

}

