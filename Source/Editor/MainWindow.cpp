#include "MainWindow.h"
#include "Configuration.h"
#include "Document.h"
#include "OptionsDialog.h"

#include <Urho3D/IO/File.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTabBar>
#include <QVBoxLayout>
#include <QtXml/QDomDocument>

namespace Urho3DEditor
{

const QString MainWindow::VarLayoutFileName = "global/layout";

MainWindow::MainWindow(Configuration& config, QMainWindow& mainWindow, Urho3D::Context& context)
    : config_(config)
    , mainWindow_(mainWindow)
    , context_(context)
    , widget_(new QWidget())
    , layout_(new QVBoxLayout())
    , tabBar_(new QTabBar())
    , urho3DWidget_(new Urho3DWidget(context_))
{
    urho3DWidget_->setVisible(false);

    config.RegisterVariable(VarLayoutFileName, ":/Layout.xml", ".Global", "Layout");
}

bool MainWindow::Initialize()
{
    InitializeLayout();
    InitializeMenu();
    return true;
}

void MainWindow::LoadLayout()
{
    const QString fileName = config_.GetValue(VarLayoutFileName).toString();
    QFile file(fileName);

    // Close if cannot load layout
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        const QMessageBox::StandardButton button = QMessageBox::critical(&mainWindow_,
            "Main Window error",
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

Configuration& MainWindow::GetConfig() const
{
    return config_;
}

Urho3D::Context& MainWindow::GetContext() const
{
    return context_;
}

Document* MainWindow::GetCurrentDocument() const
{
    return tabBar_->currentIndex() < 0 ? nullptr : documents_[tabBar_->currentIndex()];
}

Urho3DWidget* MainWindow::GetUrho3DWidget() const
{
    return urho3DWidget_.data();
}

QMenuBar* MainWindow::GetMenuBar() const
{
    return mainWindow_.menuBar();
}

QAction* MainWindow::GetAction(const QString& name) const
{
    return menuActions_.value(name);
}

QMenu* MainWindow::GetMenu(const QString& name) const
{
    return menus_.value(name);
}

QAction* MainWindow::AddAction(const QString& name, QAction* action)
{
    action->setParent(this);
    menuActions_[name] = action;
    return action;
}

QAction* MainWindow::AddAction(const QString& name, const QKeySequence& shortcut /*= QKeySequence()*/)
{
    QScopedPointer<QAction> action(new QAction);
    action->setShortcut(shortcut);
    return AddAction(name, action.take());
}

void MainWindow::AddDock(Qt::DockWidgetArea area, QDockWidget* dock)
{
    mainWindow_.addDockWidget(area, dock);
}

void MainWindow::AddDocument(Document* document, bool bringToTop /*= true*/)
{
    // Add new tab
    connect(document, SIGNAL(titleChanged(Document*)), this, SLOT(HandleTabTitleChanged(Document*)));
    document->setVisible(false);
    layout_->addWidget(document);
    documents_.push_back(document);
    const int index = tabBar_->addTab(document->GetTitle());

    // Emit signal for first tab
    if (documents_.size() == 1)
        HandleTabChanged(tabBar_->currentIndex());

    // Activate
    if (bringToTop)
        SelectDocument(document);
}

void MainWindow::SelectDocument(Document* document)
{
    const int index = documents_.indexOf(document);
    tabBar_->setCurrentIndex(index);
}

void MainWindow::CloseDocument(Document* document)
{
    emit documentClosed(document);

    const int index = documents_.indexOf(document);
    documents_.remove(index);
    tabBar_->removeTab(index);

    if (documents_.isEmpty())
    {
        // Emit signal if all tabs are closed now
        emit currentDocumentChanged(nullptr);

        // Ensure that Urho3D widget is invisible
        urho3DWidget_->setVisible(false);
    }
}

void MainWindow::InitializeLayout()
{
    widget_->setLayout(layout_.data());

    layout_->addWidget(tabBar_.data());
    layout_->addWidget(urho3DWidget_.data());

    tabBar_->setMovable(true);
    tabBar_->setDocumentMode(true);
    tabBar_->setExpanding(false);
    tabBar_->setTabsClosable(true);

    mainWindow_.setCentralWidget(widget_.data());

    connect(tabBar_.data(), SIGNAL(currentChanged(int)), this, SLOT(HandleTabChanged(int)));
    connect(tabBar_.data(), SIGNAL(tabMoved(int, int)), this, SLOT(HandleTabMoved(int, int)));
    connect(tabBar_.data(), SIGNAL(tabCloseRequested(int)), this, SLOT(HandleTabClosed(int)));
}

void MainWindow::InitializeMenu()
{
    QAction* action = nullptr;

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

QMenu* MainWindow::ReadMenu(const QDomNode& node)
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

QAction* MainWindow::ReadAction(const QDomNode& node)
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

void MainWindow::HandleFileClose()
{
    if (tabBar_->currentIndex() != -1)
        CloseDocument(documents_[tabBar_->currentIndex()]);
}

void MainWindow::HandleFileExit()
{
    mainWindow_.close();
}

void MainWindow::EditUndo()
{
    if (Document* document = GetCurrentDocument())
        document->Undo();
}

void MainWindow::EditRedo()
{
    if (Document* document = GetCurrentDocument())
        document->Redo();
}

void MainWindow::HandleToolsOptions()
{
    OptionsDialog dialog(config_);
    dialog.exec();
}

void MainWindow::HandleHelpAbout()
{
    QMessageBox messageBox;
    messageBox.setText("Urho3D Editor v0.0");
    messageBox.setInformativeText("I promise it will be better...");
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.exec();
}

void MainWindow::HandleTabChanged(int index)
{
    if (index < 0)
        return;
    Q_ASSERT(index < documents_.size());

    // Hide all documents
    for (Document* document : documents_)
        document->setVisible(false);

    // Show document
    Document* document = documents_[index];
    document->setVisible(document->IsDocumentWidgetVisible());
    urho3DWidget_->setVisible(document->IsUrho3DWidgetVisible());
    if (document->IsUrho3DWidgetVisible())
        urho3DWidget_->setFocus(Qt::ActiveWindowFocusReason);

    emit currentDocumentChanged(document);
}

void MainWindow::HandleTabMoved(int from, int to)
{
    documents_.move(from, to);
}

void MainWindow::HandleTabClosed(int index)
{
    CloseDocument(documents_[index]);
}

void MainWindow::HandleTabTitleChanged(Document* document)
{
    const int index = documents_.indexOf(document);
    tabBar_->setTabText(index, document->GetTitle());
}

void MainWindow::HandleMenuAboutToShow()
{
    if (QMenu* menu = dynamic_cast<QMenu*>(sender()))
        emit updateMenu(menu);
}

}
