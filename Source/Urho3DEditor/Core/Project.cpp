#include "Project.h"
#include "../Core/Core.h"
#include "../Core/QtUrhoHelpers.h"
#include <Urho3D/Engine/EngineDefs.h>
#include <QtXml/QDomDocument>
#include <QXmlStreamWriter>
#include <QDirIterator>
#include <QFileInfo>

namespace Urho3DEditor
{

Project::Project()
    : resourcePrefixPaths_(".")
    , resourcePaths_("CoreData;Data")
    , autoloadPaths_("Autoload")
    , defaultRenderPath_("RenderPaths/Forward.xml")
{

}

bool Project::Save()
{
    QFile file(fileName_);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return false;

    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("project");
    {
        xml.writeTextElement("resourceprefixpaths", resourcePrefixPaths_);
        xml.writeTextElement("resourcepaths", resourcePaths_);
    }
    xml.writeEndElement();
    xml.writeEndDocument();

    return true;
}

bool Project::Load()
{
    QFile file(fileName_);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    // Read layout
    QDomDocument xml;
    if (!xml.setContent(&file))
        return false;

    const QDomElement root = xml.documentElement();
    resourcePrefixPaths_ = root.namedItem("resourceprefixpaths").toElement().text();
    resourcePaths_ = root.namedItem("resourcepaths").toElement().text();

    return true;
}

Urho3D::VariantMap Project::GetResourceCacheParameters() const
{
    Urho3D::VariantMap result;
    result[Urho3D::EP_RESOURCE_PREFIX_PATHS] = Cast(GetAbsoluteResourcePrefixPaths(GetBasePath()));
    result[Urho3D::EP_RESOURCE_PATHS] = Cast(resourcePaths_);
    result[Urho3D::EP_RESOURCE_PACKAGES] = Cast(packagePaths_);
    result[Urho3D::EP_AUTOLOAD_PATHS] = Cast(autoloadPaths_);
    return result;
}

QString Project::ConcatenateList(const QStringList& list, QChar separator)
{
    QString result;
    for (int i = 0; i < list.count(); ++i)
    {
        result += list[i];
        if (i + 1 != list.count())
            result += separator;
    }
    return result;
}

QString Project::GetBasePath() const
{
    return QFileInfo(fileName_).absolutePath();
}

void Project::SetResourcePrefixPaths(const QString& prefixPaths)
{
    resourcePrefixPaths_ = prefixPaths;
}

void Project::SetResourcePaths(const QString& paths)
{
    resourcePaths_ = paths;
}

QStringList Project::GetAbsoluteResourcePrefixPathsList(const QString& basePath) const
{
    QStringList paths = GetResourcePrefixPathsList();
    for (QString& path : paths)
        path = QDir::cleanPath(QDir(basePath).filePath(path));
    return paths;
}

QString Project::GetAbsoluteResourcePrefixPaths(const QString& basePath) const
{
    return Project::ConcatenateList(GetAbsoluteResourcePrefixPathsList(basePath));
}

QStringList Project::GetResourcePrefixPathsList() const
{
    return resourcePrefixPaths_.split(';', QString::SkipEmptyParts);
}

QStringList Project::GetResourcePathsList() const
{
    return resourcePaths_.split(';', QString::SkipEmptyParts);
}

QStringList Project::GetAvailableResourcePathsList(const QString& basePath) const
{
    const QStringList prefixPaths = GetAbsoluteResourcePrefixPathsList(basePath);
    QStringList result;
    for (const QString& prefixPath : prefixPaths)
    {
        QDir path(prefixPath);
        path.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
        QDirIterator iter(path);
        while (iter.hasNext())
            result.push_back(path.relativeFilePath(iter.next()));
    }
    result.sort();
    result.removeDuplicates();
    return result;
}

}
