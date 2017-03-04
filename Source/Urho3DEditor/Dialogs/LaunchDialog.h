#pragma once

#include <QDialog>

class QGridLayout;
class QListWidget;

namespace Urho3DEditor
{

class Core;

/// Launch Dialog Widget.
class LaunchDialog : public QDialog
{
    Q_OBJECT

public:
    /// Register variables.
    static void RegisterGlobalVariables(Core& core);
    /// Construct.
    LaunchDialog(Core& core);

private slots:
    /// Create new project.
    void NewProject();
    /// Browse existing project.
    void BrowseProject();
    /// Open recent project.
    void OpenRecentProject();

private:
    /// Core.
    Core& core_;
    /// Layout.
    QGridLayout* layout_ = nullptr;
    /// Recent projects list.
    QListWidget* recentProjects_ = nullptr;

};

}
