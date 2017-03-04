#include "Core.h"
#include "Document.h"
#include "DocumentWindow.h"
#include "QtUrhoHelpers.h"
#include "GlobalVariable.h"
#include "../Configuration.h"
#include "../OptionsDialog.h"
#include "../Dialogs/LaunchDialog.h"
#include "../Documents/ProjectDocument.h"

#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <QFileDialog>
#include <QFileInfo>
#include <QCloseEvent>
#include <QMdiArea>
#include <QMenuBar>
#include <QTabBar>
#include <QVBoxLayout>
#include <QtXml/QDomDocument>

#include <QLabel>

namespace Urho3DEditor
{

static GlobalVariableT<QString> VarLayout("global/layout", ":/Layout.xml", ".Global", QT_TR_NOOP("Layout"));
static GlobalVariableT<QString> VarLastOpenedProject("project/lastopened", "");

Core::Core(Configuration& config, QMainWindow& mainWindow)
    : settings_("Urho3D", "Editor")
    , config_(config)
    , mainWindow_(mainWindow)
    , applicationParameters_(Urho3D::Engine::ParseParameters(Urho3D::GetArguments()))
    , mdiArea_(new QMdiArea(&mainWindow_))
    , urhoHost_(new Urho3DHost(&mainWindow_))
{
    // Setup widgets
    mdiArea_->setViewMode(QMdiArea::TabbedView);
    mdiArea_->setTabsMovable(true);
    mdiArea_->setTabsClosable(true);
    mainWindow_.setCentralWidget(mdiArea_);

    connect(mdiArea_, &QMdiArea::subWindowActivated, this,
        [this](QMdiSubWindow* subWindow) { ChangeDocument(qobject_cast<DocumentWindow*>(subWindow)); });

    // Register internal things
    RegisterGlobalVariable(VarLayout);
    RegisterGlobalVariable(VarLastOpenedProject);
    LaunchDialog::RegisterGlobalVariables(*this);
}

Core::~Core()
{
    SaveGlobalVariables();
    delete mdiArea_;
    delete urhoHost_;
}

QMessageBox::StandardButton Core::Error(const QString& text,
    QMessageBox::StandardButtons buttons /*= QMessageBox::Ok*/, QMessageBox::StandardButton defaultButton /*= QMessageBox::Ok*/)
{
    return QMessageBox::critical(&mainWindow_, tr("Urho3D Editor Error"), text, buttons, defaultButton);
}

bool Core::NewProject()
{
    if (currentProject_)
        return false;

    QSharedPointer<Project> project(new Project());

    // Launch save dialog
    QFileDialog dialog;
    dialog.selectFile("Project.urho");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(QFileInfo(VarLastOpenedProject.GetValue()).absolutePath());
    dialog.setNameFilter(tr("Urho3D Project (*.urho)"));
    if (!dialog.exec() || dialog.selectedFiles().isEmpty())
        return false;

    // Save project
    const QString fileName = dialog.selectedFiles().front();
    project->SetFileName(fileName);
    if (!project->Save())
    {
        Error(tr("Failed to save project %1").arg(fileName));
        return false;
    }

    VarLastOpenedProject.SetValue(fileName);
    SetCurrentProject(project);
    return true;
}

bool Core::OpenProject(QString fileName /*= ""*/)
{
    if (currentProject_)
        return false;

    QSharedPointer<Project> project(new Project());

    if (fileName.isEmpty())
    {
        // Launch open dialog
        QFileDialog dialog;
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setOption(QFileDialog::DontUseNativeDialog, true);
        dialog.setDirectory(QFileInfo(VarLastOpenedProject.GetValue()).absolutePath());
        dialog.setNameFilter(tr("Urho3D Project (*.urho)"));
        if (!dialog.exec() || dialog.selectedFiles().isEmpty())
            return false;

        // Get file name
        fileName = dialog.selectedFiles().front();
    }

    // Open project
    project->SetFileName(fileName);
    if (!project->Load())
    {
        Error(tr("Failed to load project %1").arg(fileName));
        return false;
    }

    VarLastOpenedProject.SetValue(fileName);
    SetCurrentProject(project);
    return true;
}

DocumentFactory* Core::GetDocumentFactory(const QString& documentType) const
{
    return registeredDocuments_.Get(documentType).data();
}

DocumentFactory* Core::GetDocumentFactory(Document& document) const
{
    return GetDocumentFactory(document.metaObject()->className());
}

bool Core::RegisterDocument(DocumentFactory* factory)
{
    if (!registeredDocuments_.Insert(factory->GetDocumentType(), QSharedPointer<DocumentFactory>(factory)))
    {
        Error(tr("Document %1 is already registered").arg(factory->GetDocumentType()));
        return false;
    }

    for (QString filter : factory->GetFileNameFilters())
    {
        if (filter.contains(";;"))
        {
            Error(tr("Document %1 has invalid filter %2: ';;' is not allowed").arg(factory->GetDocumentType(), filter));
            continue;
        }
        if (filterToDocumentType_.contains(filter))
        {
            Error(tr("Document %1 has already used file name filter %2").arg(factory->GetDocumentType(), filter));
            continue;
        }

        filterToDocumentType_.insert(filter, { factory->GetDocumentType() });
        registeredDocumentFilters_.push_back(filter);
    }

    return true;
}

bool Core::RegisterFilter(const QString& filter, const QStringList& documentTypes)
{
    if (filterToDocumentType_.contains(filter))
    {
        Error(tr("Filter %1 is already added").arg(filter));
        return false;
    }

    filterToDocumentType_.insert(filter, documentTypes);
    registeredDocumentFilters_.push_back(filter);
    return true;
}

void Core::RegisterGlobalVariable(GlobalVariable& variable)
{
    variable.SetContext(settings_);
    globalVariables_.push_back(&variable);
}

void Core::SaveGlobalVariables()
{
    for (GlobalVariable* variable : globalVariables_)
        variable->Save();
}

bool Core::NewDocument(const QString& documentType)
{
    const DocumentFactory* factory = GetDocumentFactory(documentType);
    if (!factory)
    {
        Error(tr("Document %1 is not registered").arg(documentType));
        return false;
    }

    QScopedPointer<Document> document(factory->CreateDocument(*this));
    assert(document);
    if (factory->IsSaveable() && factory->ShallSaveOnCreate() && !SaveDocument(*document, true))
        return false;

    AddDocument(document.take());
    return true;
}

bool Core::OpenDocument(const QString& fileName, QStringList documentTypes /*= QStringList()*/)
{
    if (fileName.isEmpty())
    {
        Error(tr("File name mustn't be empty"));
        return false;
    }

    // Fill types if empty
    if (documentTypes.isEmpty())
    {
        for (const auto& elem : registeredDocuments_)
            documentTypes.push_back(elem.first);
    }

    // Try to load document
    for (const QString& documentType : documentTypes)
    {
        const DocumentFactory* factory = GetDocumentFactory(documentType);
        if (!factory)
        {
            Error(tr("Document %1 is not registered").arg(documentType));
            return false;
        }

        QScopedPointer<Document> document(factory->CreateDocument(*this));
        assert(document);
        if (document->Open(fileName))
        {
            AddDocument(document.take());
            return true;
        }
    }
    Error(tr("Cannot open document %1").arg(fileName));
    return false;
}

bool Core::OpenDocumentDialog(const QString& documentType, bool allowMultiselect)
{
    // Initialize dialog
    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(GetConfig().GetLastDirectory());

    // Prepare filters.
    if (documentType.isEmpty())
    {
        dialog.setNameFilters(registeredDocumentFilters_);
    }
    else
    {
        const DocumentFactory* factory = GetDocumentFactory(documentType);
        if (!factory)
        {
            Error(tr("Document %1 is not registered").arg(documentType));
            return false;
        }
        dialog.setNameFilters(factory->GetFileNameFilters());
    }

    if (!dialog.exec())
        return false;

    const QStringList fileNames = dialog.selectedFiles();
    if (fileNames.isEmpty())
        return false;

    QStringList selectedType = { documentType };
    if (documentType.isEmpty())
        selectedType = filterToDocumentType_.value(dialog.selectedNameFilter(), {});

    GetConfig().SetLastDirectoryByFileName(fileNames[0]);
    if (allowMultiselect)
        for (const QString& fileName : fileNames)
            OpenDocument(fileName, selectedType);
    else
        OpenDocument(fileNames[0], selectedType);
    return true;
}

bool Core::SaveDocument(Document& document, bool saveAs /*= false*/)
{
    const DocumentFactory* factory = GetDocumentFactory(document);
    if (!factory)
    {
        Error(tr("Document %1 is not registered").arg(document.metaObject()->className()));
        return false;
    }

    if (!document.GetFileName().isEmpty() && !saveAs)
    {
        if (document.Save(document.GetFileName()))
            return true;
        Error(tr("Failed to save %1, try different location").arg(document.GetFileName()));
    }

    // Save as
    QFileDialog dialog;
    dialog.selectFile(factory->GetDefaultFileName());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(GetConfig().GetLastDirectory());
    dialog.setNameFilters(factory->GetFileNameFilters());
    if (!dialog.exec())
        return false;

    const QStringList files = dialog.selectedFiles();
    if (files.isEmpty())
        return false;

    if (!document.Save(files[0]))
    {
        Error(tr("Failed to save %1").arg(document.GetFileName()));
        return false;
    }
    return true;
}

bool Core::CloseDocument(DocumentWindow& documentWindow)
{
    // Try to save document first
    Document& document = documentWindow.GetDocument();
    if (document.IsDirty())
    {
        const QMessageBox::StandardButton result =
            QMessageBox::warning(&mainWindow_,
                tr("Urho3D Editor"),
                tr("Document %1 has unsaved changes").arg(document.GetTitle()),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);

        switch (result)
        {
        case QMessageBox::Save:
            if (!SaveDocument(document))
                return false;
            break;
        case QMessageBox::Discard:
            break;
        case QMessageBox::Cancel:
        default:
            return false;
        }
    }

    // Close document
    if (currentDocument_ == &documentWindow)
    {
        currentDocument_ = nullptr;
        emit currentDocumentChanged(nullptr);
    }
    emit documentClosed(&document);
    delete &documentWindow;
    return true;
}

void Core::SetCurrentProject(QSharedPointer<Project> project)
{
    currentProject_ = project;

    if (currentProject_)
    {
        Urho3DWidget& widget = urhoHost_->GetWidget();
        widget.SetResourceCache(currentProject_->GetResourceCacheParameters());
        widget.SetDefaultRenderPath(currentProject_->GetDefaultRenderPath());
    }
}

bool Core::Initialize()
{
    // Initialize Urho3D
    if (!urhoHost_->Initialize(applicationParameters_))
    {
        // Try redundant parameters
        applicationParameters_[Urho3D::EP_RESOURCE_PREFIX_PATHS] = Cast(QDir::currentPath());
        applicationParameters_[Urho3D::EP_RESOURCE_PATHS] = ".";
        applicationParameters_[Urho3D::EP_RESOURCE_PACKAGES] = "";
        applicationParameters_[Urho3D::EP_AUTOLOAD_PATHS] = "";
        if (!urhoHost_->Initialize(applicationParameters_))
        {
            Error(tr("Failed to initialize Urho3D"));
            return false;
        }
    }

    // Initialize menu and layout
    InitializeMenu();
    LoadLayout();

    // Initialize project
    LaunchDialog dialog(*this);
    if (dialog.exec() == QDialog::Rejected)
        return false;

    // Open main window
    mainWindow_.showMaximized();
    return true;
}

void Core::LoadLayout()
{
    QFile file(VarLayout.GetValue());

    // Close if cannot load layout
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        const QMessageBox::StandardButton result = Error(
            tr("Failed to load layout $1\nReset to default?").arg(VarLayout.GetValue()),
            QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::Yes)
            VarLayout.ResetToDefault();

        mainWindow_.close();
        return;
    }

    // Read layout
    QDomDocument xml;
    if (!xml.setContent(&file))
        return;

    const QDomElement root = xml.documentElement();
    const QDomNodeList children = root.childNodes();
    for (int i = 0; i < children.count(); ++i)
    {
        const QDomNode node = children.at(i);
        if (node.nodeName() == "menu")
        {
            const QString tag = node.attributes().namedItem("tag").nodeValue();
            if (tag.isEmpty())
            {
                const QDomNodeList menus = node.childNodes();
                for (int j = 0; j < menus.count(); ++j)
                    mainWindow_.menuBar()->addMenu(ReadMenu(menus.at(j)));
            }
            else
            {
                if (!menus_.contains(tag))
                    menus_[tag] = ReadMenu(node);
            }
        }
    }
}

Urho3DClientWidget* Core::CreateUrho3DClientWidget(QWidget* parent /*= nullptr*/)
{
    return new Urho3DClientWidget(*urhoHost_, parent);
}

Configuration& Core::GetConfig() const
{
    return config_;
}

Document* Core::GetCurrentDocument() const
{
    if (QMdiSubWindow* subWindow = mdiArea_->activeSubWindow())
    {
        if (DocumentWindow* document = qobject_cast<DocumentWindow*>(subWindow))
        {
            return &document->GetDocument();
        }
    }
    return nullptr;
}

Urho3DWidget& Core::GetUrho3DWidget() const
{
    return urhoHost_->GetWidget();
}

QMenuBar* Core::GetMenuBar() const
{
    return mainWindow_.menuBar();
}

QAction* Core::GetAction(const QString& name) const
{
    return menuActions_.value(name);
}

QMenu* Core::GetMenu(const QString& name) const
{
    return menus_.value(name);
}

QAction* Core::AddAction(const QString& name, QAction* action)
{
    action->setParent(this);
    menuActions_[name] = action;
    return action;
}

QAction* Core::AddAction(const QString& name, const QKeySequence& shortcut /*= QKeySequence()*/)
{
    QScopedPointer<QAction> action(new QAction);
    action->setShortcut(shortcut);
    return AddAction(name, action.take());
}

void Core::AddDock(Qt::DockWidgetArea area, QDockWidget* dock)
{
    mainWindow_.addDockWidget(area, dock);
}

void Core::AddDocument(Document* document, bool bringToTop /*= true*/)
{
    // #TODO Do something with bringToTop

    DocumentWindow* widget = new DocumentWindow(*this, document, &mainWindow_);
    mdiArea_->addSubWindow(widget);
    widget->show();
}

void Core::ChangeDocument(DocumentWindow* widget)
{
    if (currentDocument_ != widget)
    {
        currentDocument_ = widget;
        emit currentDocumentChanged(widget ? &widget->GetDocument() : nullptr);
    }
}

void Core::InitializeMenu()
{
    QAction* action = nullptr;

    action = AddAction("File.NewProject");
    connect(action, &QAction::triggered, this, &Core::NewDocument<ProjectDocument>);

    action = AddAction("File.Open", Qt::CTRL + Qt::Key_O);
    connect(action, &QAction::triggered, this, &Core::Open);

    action = AddAction("File.Save", Qt::CTRL + Qt::Key_S);

    action = AddAction("File.SaveAs", Qt::CTRL + Qt::SHIFT + Qt::Key_S);

    action = AddAction("File.Close", Qt::CTRL + Qt::Key_W);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(HandleFileClose()));

    action = AddAction("File.Exit");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(HandleFileExit()));

    action = AddAction("Edit.Undo", Qt::CTRL + Qt::Key_Z);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(EditUndo()));

    action = AddAction("Edit.Redo", Qt::CTRL + Qt::Key_Y);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(EditRedo()));

    action = AddAction("Tools.Options");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(HandleToolsOptions()));

    action = AddAction("Help.About");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(HandleHelpAbout()));
}

QMenu* Core::ReadMenu(const QDomNode& node)
{
    const QDomNamedNodeMap attributes = node.attributes();
    const QString name = attributes.namedItem("name").nodeValue();
    QScopedPointer<QMenu> menu(new QMenu(name, &mainWindow_));

    QDomNodeList children = node.childNodes();
    for (int i = 0; i < children.size(); ++i)
    {
        const QDomNode child = children.at(i);
        const QString type = child.nodeName();
        if (type == "menu")
            menu->addMenu(ReadMenu(child));
        else if (type == "action")
            menu->addAction(ReadAction(child));
        else if (type == "separator")
            menu->addSeparator();
    }

    connect(menu.data(), SIGNAL(aboutToShow()), this, SLOT(HandleMenuAboutToShow()));
    return menu.take();
}

QAction* Core::ReadAction(const QDomNode& node)
{
    const QDomNamedNodeMap attributes = node.attributes();
    const QString name = attributes.namedItem("name").nodeValue();
    const QString actionName = attributes.namedItem("action").nodeValue();

    QAction* action = menuActions_.value(actionName);
    if (!action)
        action = new QAction(name + " (Dummy)", &mainWindow_);
    else
        action->setText(name);
    return action;
}

void Core::HandleFileExit()
{
    mainWindow_.close();
}

void Core::EditUndo()
{
    if (Document* document = GetCurrentDocument())
        document->Undo();
}

void Core::EditRedo()
{
    if (Document* document = GetCurrentDocument())
        document->Redo();
}

void Core::HandleToolsOptions()
{
    OptionsDialog dialog(*this);
    dialog.exec();
}

void Core::HandleHelpAbout()
{
    QMessageBox messageBox;
    messageBox.setText("Urho3D Editor v0.0");
    messageBox.setInformativeText("I promise it will be better...");
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.exec();
}

void Core::HandleMenuAboutToShow()
{
    if (QMenu* menu = dynamic_cast<QMenu*>(sender()))
        emit updateMenu(menu);
}

}
