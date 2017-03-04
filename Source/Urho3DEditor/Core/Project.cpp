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

QString Project::GetTitle() const
{
    return QFileInfo(fileName_).fileName();
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
    result[Urho3D::EP_RESOURCE_PREFIX_PATHS] = Cast(GetAbsoluteResourcePrefixPaths());
    result[Urho3D::EP_RESOURCE_PATHS] = Cast(resourcePaths_);
    result[Urho3D::EP_RESOURCE_PACKAGES] = Cast(packagePaths_);
    result[Urho3D::EP_AUTOLOAD_PATHS] = Cast(autoloadPaths_);
    return result;
}

QString Project::GetAbsoluteResourcePrefixPaths() const
{
    // Get base path
    const QString basePath = QFileInfo(fileName_).absolutePath();

    // Get list of prefix paths
    QStringList prefixPaths = resourcePrefixPaths_.split(';', QString::SkipEmptyParts);

    // Make absolute paths
    for (QString& path : prefixPaths)
        path = QDir::cleanPath(QDir(basePath).filePath(path));

    // Join all strings
    return prefixPaths.join(';');
}

}
