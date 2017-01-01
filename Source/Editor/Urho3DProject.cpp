#include "Urho3DProject.h"
#include "MainWindow.h"
// #include "AbstractDocument.h"
// #include "EditorSettings.h"
// #include "Urho3DWidget.h"
// #include <Urho3D/Core/CoreEvents.h>
// #include <Urho3D/Graphics/Camera.h>
// #include <Urho3D/Graphics/Renderer.h>
// #include <Urho3D/Input/Input.h>
// #include <Urho3D/IO/IOEvents.h>
#include <Urho3D/IO/File.h>
// #include <Urho3D/Scene/Scene.h>
// #include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Resource/XMLFile.h>
// #include <QVBoxLayout>
// #include <QFileDialog>
#include <QDirIterator>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>

namespace Urho3DEditor
{

ProjectManager::ProjectManager()
    : mainWindow_(nullptr)
{

}

bool ProjectManager::DoInitialize()
{
    mainWindow_ = GetModule<MainWindow>();
    if (!mainWindow_)
        return false;

    QMenu* menuFile = mainWindow_->GetTopLevelMenu(MainWindow::MenuFile);
    QAction* menuFileNew_After = mainWindow_->GetMenuAction(MainWindow::MenuFileNew_After);
    if (!menuFile || !menuFileNew_After)
        return false;

    actionFileNewProject_.reset(new QAction("New Project"));
    menuFile->insertAction(menuFileNew_After, actionFileNewProject_.data());
    connect(actionFileNewProject_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleFileNewProject()));

    return true;
}


void ProjectManager::HandleFileNewProject()
{

}

//////////////////////////////////////////////////////////////////////////
Urho3DProject::Urho3DProject(Urho3D::Context* context)
    : Urho3D::Resource(context)
    , resourcePrefixPaths_(".")
    , resourcePaths_("CoreData;Data")
{

}

QString Urho3DProject::ConcatenateList(const QStringList& list, QChar separator)
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

QString Urho3DProject::GetBasePath() const
{
    return QFileInfo(GetName().CString()).absolutePath();
}

void Urho3DProject::SetResourcePrefixPaths(const QString& prefixPaths)
{
    resourcePrefixPaths_ = prefixPaths;
}

void Urho3DProject::SetResourcePaths(const QString& paths)
{
    resourcePaths_ = paths;
}

QStringList Urho3DProject::GetAbsoluteResourcePrefixPathsList(const QString& basePath) const
{
    QStringList paths = GetResourcePrefixPathsList();
    for (QString& path : paths)
        path = QDir::cleanPath(QDir(basePath).filePath(path));
    return paths;
}

QString Urho3DProject::GetAbsoluteResourcePrefixPaths(const QString& basePath) const
{
    return Urho3DProject::ConcatenateList(GetAbsoluteResourcePrefixPathsList(basePath));
}

QStringList Urho3DProject::GetResourcePrefixPathsList() const
{
    return resourcePrefixPaths_.split(';', QString::SkipEmptyParts);
}

QStringList Urho3DProject::GetResourcePathsList() const
{
    return resourcePaths_.split(';', QString::SkipEmptyParts);
}

QStringList Urho3DProject::GetAvailableResourcePathsList(const QString& basePath) const
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

bool Urho3DProject::BeginLoad(Urho3D::Deserializer& source)
{
    using namespace Urho3D;

    XMLFile xmlFile(context_);
    if (!xmlFile.Load(source))
        return false;

    XMLElement root = xmlFile.GetRoot("project");
    resourcePrefixPaths_ = root.GetChild("resourceprefixpaths").GetValue().CString();
    resourcePaths_ = root.GetChild("resourcepaths").GetValue().CString();
    return true;
}

bool Urho3DProject::Save(Urho3D::Serializer& dest) const
{
    using namespace Urho3D;

    XMLFile xmlFile(context_);
    XMLElement root = xmlFile.CreateRoot("project");
    root.CreateChild("resourceprefixpaths").SetValue(resourcePrefixPaths_.toStdString().c_str());
    root.CreateChild("resourcepaths").SetValue(resourcePaths_.toStdString().c_str());
    return xmlFile.Save(dest);
}

//////////////////////////////////////////////////////////////////////////
Urho3DProjectPage::Urho3DProjectPage(Urho3D::Context* context)
    : AbstractPage(context)
    , project_(Urho3D::MakeShared<Urho3DProject>(context))
    , layout_(new QGridLayout())
    , buttonSetAsCurrent_(new QPushButton())
    , fieldResourcePrefixPaths_(new QLineEdit("."))
    , fieldResourcePaths_(new QLineEdit("CoreData;Data"))
{
    SetTitle("New project");
    MarkUnsaved();

    setLayout(layout_);
    connect(fieldResourcePrefixPaths_, SIGNAL(textEdited(const QString&)), this, SLOT(OnResourcePrefixPathsEdited(const QString&)));
    connect(fieldResourcePaths_,       SIGNAL(textEdited(const QString&)), this, SLOT(OnResourcePathsEdited(const QString&)));

    layout_->addWidget(buttonSetAsCurrent_,                  0, 0);
    layout_->addWidget(new QLabel("Resource Prefix Paths:"), 1, 0);
    layout_->addWidget(fieldResourcePrefixPaths_,            1, 1);
    layout_->addWidget(new QLabel("Resource Paths:"),        2, 0);
    layout_->addWidget(fieldResourcePaths_,                  2, 1);
    layout_->setRowStretch(3, 1);
}

QString Urho3DProjectPage::GetFilters() const
{
    return "Urho3D Project (*.urho);;All files (*.*)";
}

bool Urho3DProjectPage::DoSave()
{
    project_->SetResourcePrefixPaths(fieldResourcePrefixPaths_->text());
    project_->SetResourcePaths(fieldResourcePaths_->text());

    const Urho3D::String fileName = GetFileName().toStdString().c_str();
    project_->SetName(fileName);
    return project_->SaveFile(fileName);
}

bool Urho3DProjectPage::DoLoad()
{
    const Urho3D::String fileName = GetFileName().toStdString().c_str();
    if (!project_->LoadFile(fileName))
        return false;
    project_->SetName(fileName);

    fieldResourcePrefixPaths_->setText(project_->GetResourcePrefixPaths());
    fieldResourcePaths_->setText(project_->GetResourcePaths());
    return true;
}

void Urho3DProjectPage::OnResourcePrefixPathsEdited(const QString& value)
{
    MarkUnsaved();
}

void Urho3DProjectPage::OnResourcePathsEdited(const QString& value)
{
    MarkUnsaved();
}

}
