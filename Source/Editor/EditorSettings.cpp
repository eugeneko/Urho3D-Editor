#include "EditorSettings.h"
#include <QFileInfo>

namespace Urho3D
{

const QString EditorSettings::LAST_DIRECTORY = "LastDirectory";
const QString EditorSettings::RECENT_PROJECTS = "RecentProjects";

EditorSettings::EditorSettings(Context* context)
    : Object(context)
    , settings_("Urho3D", "Editor")
{
}

void EditorSettings::SetLastDirectory(const QString& value)
{
    settings_.setValue(LAST_DIRECTORY, QFileInfo(value).absolutePath());
}

void EditorSettings::AddRecentProject(const QString& name)
{
    QStringList recentProjects = GetRecentProjects();
    recentProjects.removeAll(name);
    recentProjects.push_front(name);
    settings_.setValue(RECENT_PROJECTS, recentProjects);
}

}
