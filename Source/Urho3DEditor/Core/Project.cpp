#include "Project.h"
#include "../Core/Core.h"
#include "../Core/QtUrhoHelpers.h"
#include <Urho3D/IO/File.h>
#include <Urho3D/Resource/XMLFile.h>
// #include <QtXml/QDomDocument>
#include <QAction>
#include <QDirIterator>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>

namespace Urho3DEditor
{

Project::Project(Urho3D::Context* context)
    : Urho3D::Resource(context)
    , resourcePrefixPaths_(".")
    , resourcePaths_("CoreData;Data")
{

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
    return QFileInfo(GetName().CString()).absolutePath();
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

bool Project::BeginLoad(Urho3D::Deserializer& source)
{
    using namespace Urho3D;

    XMLFile xmlFile(context_);
    if (!xmlFile.Load(source))
        return false;

    XMLElement root = xmlFile.GetRoot("project");
    if (root.IsNull())
        return false;

    resourcePrefixPaths_ = root.GetChild("resourceprefixpaths").GetValue().CString();
    resourcePaths_ = root.GetChild("resourcepaths").GetValue().CString();
    return true;
}

bool Project::Save(Urho3D::Serializer& dest) const
{
    using namespace Urho3D;

    XMLFile xmlFile(context_);
    XMLElement root = xmlFile.CreateRoot("project");
    root.CreateChild("resourceprefixpaths").SetValue(resourcePrefixPaths_.toStdString().c_str());
    root.CreateChild("resourcepaths").SetValue(resourcePaths_.toStdString().c_str());
    return xmlFile.Save(dest);
}

}
