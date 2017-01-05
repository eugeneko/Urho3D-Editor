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

Configuration::~Configuration()
{
    Save();
}

void Configuration::Save()
{
    for (QHash<QString, QVariant>::Iterator i = variables_.begin(); i != variables_.end(); ++i)
        settings_.setValue(i.key(), i.value());
}

void Configuration::RegisterVariable(const QString& key, const QVariant& defaultValue,
    const QString& comment /*= QString()*/, const QVariant& decoration /*= QVariant()*/)
{
    defaultValues_[key] = defaultValue;
    if (!comment.isEmpty())
        comments_[key] = comment;
    if (!decoration.isNull())
        decorations_[key] = decoration;
}

QVariant Configuration::GetDefaultValue(const QString& key)
{
    return defaultValues_.value(key);
}

QVariant Configuration::GetValue(const QString& key)
{
    // Load from settings or defaults
    if (!variables_.contains(key))
    {
        const QVariant value = settings_.value(key);
        if (!value.isNull())
            variables_[key] = value;
        else
            variables_[key] = defaultValues_.value(key);
    }
    return variables_[key];
}

void Configuration::SetValue(const QString& key, const QVariant& value, bool saveImmediately /*= false*/)
{
    variables_[key] = value;
    if (saveImmediately)
        settings_.setValue(key, value);
}

QString Configuration::GetComment(const QString& key) const
{
    return comments_.value(key);
}

QVariant Configuration::GetDecoration(const QString& key) const
{
    return decorations_.value(key);
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
