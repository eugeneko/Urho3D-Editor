#include "LaunchDialog.h"
#include "Core/Core.h"
#include "Core/GlobalVariable.h"
#include <QLabel>
#include <QListWidget>

namespace Urho3DEditor
{

static GlobalVariableT<QStringList> VarRecentProjects("project/recentlist", {});

void LaunchDialog::RegisterGlobalVariables(Core& core)
{
    core.RegisterGlobalVariable(VarRecentProjects);
}

LaunchDialog::LaunchDialog(Core& core)
    : QDialog()
    , core_(core)
    , layout_(new QGridLayout(this))
{
    setWindowTitle(tr("Startup Dialog"));

    QPushButton* newProject = new QPushButton(tr("New Project..."), this);
    connect(newProject, &QPushButton::clicked, this, &LaunchDialog::NewProject);
    layout_->addWidget(newProject, 0, 0);

    QPushButton* openProject = new QPushButton(tr("Open Project..."), this);
    connect(openProject, &QPushButton::clicked, this, &LaunchDialog::BrowseProject);
    layout_->addWidget(openProject, 1, 0);

    layout_->addWidget(new QLabel(tr("Recent projects:")));
    recentProjects_ = new QListWidget(this);
    connect(recentProjects_, &QListWidget::doubleClicked, this, &LaunchDialog::OpenRecentProject);
    layout_->addWidget(recentProjects_);

    for (const QString& recentProject : VarRecentProjects.GetValue())
        recentProjects_->addItem(recentProject);

    setLayout(layout_);
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void LaunchDialog::NewProject()
{
    if (core_.NewProject())
        accept();
}

void LaunchDialog::BrowseProject()
{
    if (core_.OpenProject())
    {
        if (Project* project = core_.GetProject())
        {
            const QString fileName = project->GetFileName();
            QStringList recentProjects = VarRecentProjects.GetValue();
            recentProjects.push_back(fileName);
            while (recentProjects.size() > 10)
                recentProjects.pop_front();
            VarRecentProjects.SetValue(recentProjects);
        }
        accept();
    }
}

void LaunchDialog::OpenRecentProject()
{
    QList<QListWidgetItem*> selection = recentProjects_->selectedItems();
    if (!selection.isEmpty())
    {
        const QString fileName = selection[0]->text();
        if (core_.OpenProject(fileName))
            accept();
    }
}

}
