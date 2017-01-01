#include "Configuration.h"
#include <QFileInfo>

namespace Urho3DEditor
{

const QString Configuration::CORE_LASTDIRECTORY = "Core.LastDirectory";
const QString Configuration::PROJECT_RECENT = "Project.Recent";

Configuration::Configuration()
    : settings_("Urho3D", "Editor")
{

}

void Configuration::SetLastDirectoryByFileName(const QString& fileName)
{
    SetLastDirectory(QFileInfo(fileName).absolutePath());
}

void Configuration::AddRecentProject(const QString& name)
{
    QStringList recentProjects = GetRecentProjects();
    recentProjects.removeAll(name);
    recentProjects.push_front(name);
    settings_.setValue(PROJECT_RECENT, recentProjects);
}

}
