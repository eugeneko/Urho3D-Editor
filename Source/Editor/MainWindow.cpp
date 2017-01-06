#include "MainWindow.h"
#include "Configuration.h"
#include "Document.h"
#include "OptionsDialog.h"

#include <Urho3D/IO/File.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <QFileDialog>
#include <QFileInfo>
#include <QTabBar>
#include <QMessageBox>

#include <QPushButton>
#include <QVBoxLayout>

namespace Urho3DEditor
{

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
}

bool MainWindow::Initialize()
{
    InitializeLayout();
    InitializeMenu();
    return true;
}

Configuration& MainWindow::GetConfig() const
{
    return config_;
}

Urho3D::Context& MainWindow::GetContext() const
{
    return context_;
}

Document* MainWindow::GetCurrentPage() const
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

QMenu* MainWindow::GetTopLevelMenu(TopLevelMenu menu) const
{
    return topLevelMenus_[menu];
}

QAction* MainWindow::GetMenuAction(MenuAction action) const
{
    return menuActions_.value(action, nullptr);
}

void MainWindow::AddDock(Qt::DockWidgetArea area, QDockWidget* dock)
{
    mainWindow_.addDockWidget(area, dock);
}

void MainWindow::AddPage(Document* document, bool bringToTop /*= true*/)
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
        SelectPage(document);
}

void MainWindow::SelectPage(Document* document)
{
    const int index = documents_.indexOf(document);
    tabBar_->setCurrentIndex(index);
}

void MainWindow::ClosePage(Document* document)
{
    emit pageClosed(document);

    const int index = documents_.indexOf(document);
    documents_.remove(index);
    tabBar_->removeTab(index);

    if (documents_.isEmpty())
    {
        // Emit signal if all tabs are closed now
        emit pageChanged(nullptr);

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
}

void MainWindow::InitializeMenu()
{
    QMenuBar* menuBar = mainWindow_.menuBar();
    topLevelMenus_[MenuFile] = menuBar->addMenu("File");
    topLevelMenus_[MenuView] = menuBar->addMenu("View");
    topLevelMenus_[MenuTools] = menuBar->addMenu("Tools");
    topLevelMenus_[MenuHelp] = menuBar->addMenu("Help");

    menuActions_[MenuFileNew_After]    = topLevelMenus_[MenuFile]->addSeparator();
    menuActions_[MenuFileOpen_After]   = topLevelMenus_[MenuFile]->addSeparator();
    menuActions_[MenuFileClose]        = topLevelMenus_[MenuFile]->addAction("Close");
    menuActions_[MenuFileSave]         = topLevelMenus_[MenuFile]->addAction("Save");
    menuActions_[MenuFileSaveAs]       = topLevelMenus_[MenuFile]->addAction("Save As...");
    menuActions_[MenuFileExit_Before]  = topLevelMenus_[MenuFile]->addSeparator();
    menuActions_[MenuFileExit]         = topLevelMenus_[MenuFile]->addAction("Exit");

    menuActions_[MenuToolsOptions]     = topLevelMenus_[MenuTools]->addAction("Options");

    menuActions_[MenuHelpAbout_Before] = topLevelMenus_[MenuHelp]->addSeparator();
    menuActions_[MenuHelpAbout]        = topLevelMenus_[MenuHelp]->addAction("About");

    menuActions_[MenuFileClose]->setShortcut(Qt::CTRL + Qt::Key_W);
    menuActions_[MenuFileSave]->setShortcut(Qt::CTRL + Qt::Key_S);
    menuActions_[MenuFileSaveAs]->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);

    connect(menuActions_[MenuFileClose], SIGNAL(triggered(bool)), this, SLOT(HandleFileClose()));
    connect(menuActions_[MenuFileExit],  SIGNAL(triggered(bool)), this, SLOT(HandleFileExit()));
    connect(menuActions_[MenuToolsOptions], SIGNAL(triggered(bool)), this, SLOT(HandleToolsOptions()));
    connect(menuActions_[MenuHelpAbout], SIGNAL(triggered(bool)), this, SLOT(HandleHelpAbout()));
    connect(tabBar_.data(), SIGNAL(currentChanged(int)),    this, SLOT(HandleTabChanged(int)));
    connect(tabBar_.data(), SIGNAL(tabMoved(int, int)),     this, SLOT(HandleTabMoved(int, int)));
    connect(tabBar_.data(), SIGNAL(tabCloseRequested(int)), this, SLOT(HandleTabClosed(int)));
}

void MainWindow::HandleFileClose()
{
    if (tabBar_->currentIndex() != -1)
        ClosePage(documents_[tabBar_->currentIndex()]);
}

void MainWindow::HandleFileExit()
{
    mainWindow_.close();
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
    document->setVisible(document->IsPageWidgetVisible());
    urho3DWidget_->setVisible(document->IsUrho3DWidgetVisible());
    if (document->IsUrho3DWidgetVisible())
        urho3DWidget_->setFocus(Qt::ActiveWindowFocusReason);

    emit pageChanged(document);
}

void MainWindow::HandleTabMoved(int from, int to)
{
    documents_.move(from, to);
}

void MainWindow::HandleTabClosed(int index)
{
    ClosePage(documents_[index]);
}

void MainWindow::HandleTabTitleChanged(Document* document)
{
    const int index = documents_.indexOf(document);
    tabBar_->setTabText(index, document->GetTitle());
}

}
