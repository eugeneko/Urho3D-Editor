#include "MainWindow.h"
#include "Configuration.h"
#include "Document.h"
#include "OptionsDialog.h"

#include <Urho3D/IO/File.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiArea>
#include <QMenuBar>
#include <QMessageBox>
#include <QTabBar>
#include <QVBoxLayout>
#include <QtXml/QDomDocument>

#include <QLabel>

namespace Urho3DEditor
{

DocumentWindow::DocumentWindow(Document* document, QWidget* parent /*= nullptr*/)
    : QMdiSubWindow(parent)
    , document_(document)
{
    updateTitle();
    connect(document, &Document::titleChanged, this, &DocumentWindow::updateTitle);
    setWidget(document);
}

void DocumentWindow::updateTitle()
{
    setWindowTitle(document_->GetTitle());
}

void DocumentWindow::closeEvent(QCloseEvent *event)
{
    emit aboutToClose();
}

//////////////////////////////////////////////////////////////////////////
const QString Core::VarLayoutFileName = "global/layout";

Core::Core(Configuration& config, QMainWindow& mainWindow)
    : config_(config)
    , mainWindow_(mainWindow)
    , mdiArea_(new QMdiArea(&mainWindow_))
    , urhoHost_(new Urho3DHost(&mainWindow_))
{
    mdiArea_->setViewMode(QMdiArea::TabbedView);
    mdiArea_->setTabsMovable(true);
    mdiArea_->setTabsClosable(true);
    mainWindow_.setCentralWidget(mdiArea_);

    connect(mdiArea_, &QMdiArea::subWindowActivated, this,
        [this](QMdiSubWindow* subWindow) { ChangeDocument(qobject_cast<DocumentWindow*>(subWindow)); });

    config.RegisterVariable(VarLayoutFileName, ":/Layout.xml", ".Global", "Layout");
}

Core::~Core()
{
    delete mdiArea_;
    delete urhoHost_;
}

bool Core::Initialize()
{
    InitializeMenu();
    return true;
}

void Core::LoadLayout()
{
    const QString fileName = config_.GetValue(VarLayoutFileName).toString();
    QFile file(fileName);

    // Close if cannot load layout
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        const QMessageBox::StandardButton button = QMessageBox::critical(&mainWindow_,
            "Urho3D Editor error",
            "Failed to load layout " + fileName + "\nReset to default?",
            QMessageBox::Yes | QMessageBox::No);

        if (button == QMessageBox::Yes)
            config_.SetValue(VarLayoutFileName, config_.GetDefaultValue(VarLayoutFileName), true);

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
            return document->GetDocument();
        }
    }
    return false;
}

Urho3DWidget* Core::GetUrho3DWidget() const
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

    DocumentWindow* widget = new DocumentWindow(document, &mainWindow_);
    connect(widget, &DocumentWindow::aboutToClose, this, [this, widget]() { CloseDocument(widget); });
    mdiArea_->addSubWindow(widget);
    widget->show();
}

void Core::CloseDocument(DocumentWindow* widget)
{
    if (currentDocument_ == widget)
    {
        emit currentDocumentChanged(nullptr);
        currentDocument_ = nullptr;
    }
    emit documentClosed(widget->GetDocument());
    delete widget;
}

void Core::ChangeDocument(DocumentWindow* widget)
{
    if (currentDocument_ != widget)
    {
        currentDocument_ = widget;
        emit currentDocumentChanged(widget ? widget->GetDocument() : nullptr);
    }
}

void Core::NewProject()
{
    QScopedPointer<ProjectDocument> document(new ProjectDocument(*this));
    if (document->SaveAs())
        AddDocument(document.take());
}

void Core::InitializeMenu()
{
    QAction* action = nullptr;

    action = AddAction("File.NewProject");
    connect(action, &QAction::triggered, this, &Core::NewProject);

    action = AddAction("File.Close", Qt::CTRL + Qt::Key_W);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(HandleFileClose()));

    action = AddAction("File.Save", Qt::CTRL + Qt::Key_S);

    action = AddAction("File.SaveAs", Qt::CTRL + Qt::SHIFT + Qt::Key_S);

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
    OptionsDialog dialog(config_);
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
