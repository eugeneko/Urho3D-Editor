#include "MainWindow.h"
#include "Configuration.h"

#include "AbstractDocument.h"

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

MainWindow::MainWindow(QMainWindow* mainWindow, Urho3D::Context* context)
    : mainWindow_(mainWindow)
    , context_(context)
    , centralWidget_(new QWidget())
    , layout_(new QVBoxLayout())
    , tabBar_(new QTabBar())
    , urho3DWidget_(new Urho3DWidget(context_))
{
    urho3DWidget_->setVisible(false);
}

Configuration& MainWindow::GetConfig() const
{
    return *config_;
}

Urho3D::Context* MainWindow::GetContext() const
{
    return context_;
}

MainWindowPage* MainWindow::GetCurrentPage() const
{
    return tabBar_->currentIndex() < 0 ? nullptr : pages_[tabBar_->currentIndex()];
}

Urho3DWidget* MainWindow::GetUrho3DWidget() const
{
    return urho3DWidget_.data();
}

QMenuBar* MainWindow::GetMenuBar() const
{
    return mainWindow_->menuBar();
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
    mainWindow_->addDockWidget(area, dock);
}

void MainWindow::AddPage(MainWindowPage* page, bool bringToTop /*= true*/)
{
    // Add new tab
    connect(page, SIGNAL(titleChanged(MainWindowPage*)), this, SLOT(HandleTabTitleChanged(MainWindowPage*)));
    page->setVisible(false);
    layout_->addWidget(page);
    pages_.push_back(page);
    const int index = tabBar_->addTab(page->GetTitle());

    // Emit signal for first tab
    if (pages_.size() == 1)
        HandleTabChanged(tabBar_->currentIndex());

    // Activate
    if (bringToTop)
        SelectPage(page);
}

void MainWindow::SelectPage(MainWindowPage* page)
{
    const int index = pages_.indexOf(page);
    tabBar_->setCurrentIndex(index);
}

void MainWindow::ClosePage(MainWindowPage* page)
{
    emit pageClosed(page);

    const int index = pages_.indexOf(page);
    pages_.remove(index);
    tabBar_->removeTab(index);

    if (pages_.isEmpty())
    {
        // Emit signal if all tabs are closed now
        emit pageChanged(nullptr);

        // Ensure that Urho3D widget is invisible
        urho3DWidget_->setVisible(false);
    }
}

bool MainWindow::DoInitialize()
{
    config_ = GetModule<Configuration>();
    if (!config_)
        return false;

    InitializeLayout();
    InitializeMenu();
    return true;
}

void MainWindow::InitializeLayout()
{
    centralWidget_->setLayout(layout_.data());

    layout_->addWidget(tabBar_.data());
    layout_->addWidget(urho3DWidget_.data());

    tabBar_->setMovable(true);
    tabBar_->setDocumentMode(true);
    tabBar_->setExpanding(false);
    tabBar_->setTabsClosable(true);

    mainWindow_->setCentralWidget(centralWidget_.data());
}

void MainWindow::InitializeMenu()
{
    QMenuBar* menuBar = mainWindow_->menuBar();
    topLevelMenus_[MenuFile] = menuBar->addMenu("File");
    topLevelMenus_[MenuView] = menuBar->addMenu("View");
    topLevelMenus_[MenuHelp] = menuBar->addMenu("Help");

    menuActions_[MenuFileNew_After]    = topLevelMenus_[MenuFile]->addSeparator();
    menuActions_[MenuFileOpen_After]   = topLevelMenus_[MenuFile]->addSeparator();
    menuActions_[MenuFileClose]        = topLevelMenus_[MenuFile]->addAction("Close");
    menuActions_[MenuFileSave]         = topLevelMenus_[MenuFile]->addAction("Save");
    menuActions_[MenuFileSaveAs]       = topLevelMenus_[MenuFile]->addAction("Save As...");
    menuActions_[MenuFileExit_Before]  = topLevelMenus_[MenuFile]->addSeparator();
    menuActions_[MenuFileExit]         = topLevelMenus_[MenuFile]->addAction("Exit");

    menuActions_[MenuHelpAbout_Before] = topLevelMenus_[MenuHelp]->addSeparator();
    menuActions_[MenuHelpAbout]        = topLevelMenus_[MenuHelp]->addAction("About");

    menuActions_[MenuFileClose]->setShortcut(Qt::CTRL + Qt::Key_W);
    menuActions_[MenuFileSave]->setShortcut(Qt::CTRL + Qt::Key_S);
    menuActions_[MenuFileSaveAs]->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);

    connect(menuActions_[MenuFileClose], SIGNAL(triggered(bool)), this, SLOT(HandleFileClose()));
    connect(menuActions_[MenuFileExit],  SIGNAL(triggered(bool)), this, SLOT(HandleFileExit()));
    connect(menuActions_[MenuHelpAbout], SIGNAL(triggered(bool)), this, SLOT(HandleHelpAbout()));
    connect(tabBar_.data(), SIGNAL(currentChanged(int)),    this, SLOT(HandleTabChanged(int)));
    connect(tabBar_.data(), SIGNAL(tabMoved(int, int)),     this, SLOT(HandleTabMoved(int, int)));
    connect(tabBar_.data(), SIGNAL(tabCloseRequested(int)), this, SLOT(HandleTabClosed(int)));
}

void MainWindow::HandleFileClose()
{
    if (tabBar_->currentIndex() != -1)
        ClosePage(pages_[tabBar_->currentIndex()]);
}

void MainWindow::HandleFileExit()
{
    mainWindow_->close();
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
    Q_ASSERT(index < pages_.size());

    // Hide all pages
    for (MainWindowPage* page : pages_)
        page->setVisible(false);

    // Show page
    MainWindowPage* page = pages_[index];
    page->setVisible(page->IsPageWidgetVisible());
    urho3DWidget_->setVisible(page->IsUrho3DWidgetVisible());
    if (page->IsUrho3DWidgetVisible())
        urho3DWidget_->setFocus(Qt::ActiveWindowFocusReason);

    emit pageChanged(page);
}

void MainWindow::HandleTabMoved(int from, int to)
{
    pages_.move(from, to);
}

void MainWindow::HandleTabClosed(int index)
{
    ClosePage(pages_[index]);
}

void MainWindow::HandleTabTitleChanged(MainWindowPage* page)
{
    const int index = pages_.indexOf(page);
    tabBar_->setTabText(index, page->GetTitle());
}

//////////////////////////////////////////////////////////////////////////
MainWindowPage::MainWindowPage(MainWindow& mainWindow)
    : mainWindow_(mainWindow)
{
    connect(&mainWindow, SIGNAL(pageChanged(MainWindowPage*)), this, SLOT(HandleCurrentPageChanged(MainWindowPage*)));
}

void MainWindowPage::SetTitle(const QString& title)
{
    if (title != title_)
    {
        title_ = title;
        emit titleChanged(this);
    }
}

bool MainWindowPage::LaunchFileDialog(bool open)
{
    QFileDialog dialog;
    dialog.setAcceptMode(open ? QFileDialog::AcceptOpen : QFileDialog::AcceptSave);
    dialog.setFileMode(open ? QFileDialog::ExistingFile : QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(mainWindow_.GetConfig().GetLastDirectory());
    dialog.setNameFilter(GetNameFilters());
    if (!dialog.exec())
        return false;

    const QStringList files = dialog.selectedFiles();
    if (files.isEmpty())
        return false;

    fileName_ = files[0];
    mainWindow_.GetConfig().SetLastDirectoryByFileName(fileName_);
    SetTitle(QFileInfo(fileName_).fileName());
    return true;
}

bool MainWindowPage::Open()
{
    if (LaunchFileDialog(true))
        return DoLoad(fileName_);
    return false;
}

bool MainWindowPage::IsActive() const
{
    return mainWindow_.GetCurrentPage() == this;
}

void MainWindowPage::HandleCurrentPageChanged(MainWindowPage* page)
{

}

bool MainWindowPage::DoLoad(const QString& /*fileName*/)
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
MainWindow1::MainWindow1(Urho3D::Context* context)
    : Urho3D::Object(context)
    , centralWidget_(new QWidget(this))
    , layout_(new QVBoxLayout(this))
    , tabBar_(new QTabBar(this))
    , urho3DWidget_(new Urho3DWidget(context_))
    , urho3DProject_(nullptr)
{
    SetCurrentProject(nullptr);

    setCentralWidget(centralWidget_);
    centralWidget_->setLayout(layout_);
    layout_->addWidget(tabBar_);
    layout_->addWidget(urho3DWidget_);
    AddPage(new StartPage(context), true);

    connect(tabBar_, SIGNAL(currentChanged(int)), this, SLOT(OnPageChanged(int)));

    // Create menu
    QMenu* menuFile = menuBar()->addMenu("File");

    actionNewProject_ = menuFile->addAction("New Project");
    connect(actionNewProject_, SIGNAL(triggered(bool)), this, SLOT(OnFileNewProject()));

    actionSave_ = menuFile->addAction("Save");
    connect(actionSave_, SIGNAL(triggered(bool)), this, SLOT(OnFileSave()));

    menuFile->addSeparator();

    actionOpenProject_ = menuFile->addAction("Open Project...");
    connect(actionOpenProject_, SIGNAL(triggered(bool)), this, SLOT(OnFileOpenProject()));

    actionCloseProject_ = menuFile->addAction("Close Project");
    connect(actionCloseProject_, SIGNAL(triggered(bool)), this, SLOT(OnFileCloseProject()));

    QMenu* viewMenu = menuBar()->addMenu("View");

//     connect(menuFile->addAction("New Scene"), SIGNAL(triggered(bool)), this, SLOT(OnFileNewScene()));
//     connect(menuFile->addAction("Open Scene"), SIGNAL(triggered(bool)), this, SLOT(OnFileOpenScene()));
//     connect(menuFile->addAction("Save Scene"), SIGNAL(triggered(bool)), this, SLOT(OnFileSaveScene()));
//     connect(menuFile->addAction("Exit"), SIGNAL(triggered(bool)), this, SLOT(OnFileExit()));

    menuBar()->show();

    // Create window body
    //     QWidget* windowBody = new QWidget();
    // 
    //     QVBoxLayout* windowBodyLayout = new QVBoxLayout(this);
    // 
    //     tabBar_ = new QTabBar();
    //     tabBar_->addTab("Scene1.xml");
    //     tabBar_->addTab("Scene2.xml");
    //     windowBodyLayout->addWidget(tabBar_);
    // 
    //     urho3DWidget_ = new Urho3DWidget(context_);
    //     windowBodyLayout->addWidget(urho3DWidget_);
    // 
    //     windowBody->setLayout(windowBodyLayout);

//     QWidget* centralWidget = new QWidget();
//     QVBoxLayout* layout = new QVBoxLayout();
//     layout->addWidget(tabWidget_);
//     layout->addWidget(urho3DWidget_.data());
//     centralWidget->setLayout(layout);
//     setCentralWidget(centralWidget);
//     setCentralWidget(tabWidget_);
//     tabWidget_->tabBar()->setTabsClosable(true);
//     connect(tabWidget_, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)));
//     tabWidget_->addTab(urho3DWidget_, "Scene1.xml");
//     tabWidget_->addTab(new QPushButton("Push"), "Scene2.xml");
//     auto doc1 = new QDockWidget("Dock1");
//     doc1->setWidget(new QPushButton("Push"));
//     addDockWidget(Qt::RightDockWidgetArea, doc1);
//     auto doc2 = new QDockWidget("Dock2");
//     doc2->setWidget(new QPushButton("Posh"));
//     addDockWidget(Qt::RightDockWidgetArea, doc2);
}

MainWindow1::~MainWindow1()
{
}

void MainWindow1::SetCurrentProject(Urho3D::SharedPtr<Urho3DProject> project)
{
    urho3DProject_ = project;
    urho3DWidget_->SetCurrentProject(urho3DProject_);
    static const QString title = "Urho3D Editor";
    if (urho3DProject_)
        setWindowTitle(title + ": " + urho3DProject_->GetName().CString());
    else
        setWindowTitle(title + ": No current project");
}

AbstractPage* MainWindow1::GetCurrentPage() const
{
    return tabBar_->currentIndex() < (int)pages_.Size() ? pages_[tabBar_->currentIndex()] : nullptr;
}

// bool MainWindow::OpenProject(const QString& fileName)
// {
//     SharedPtr<Urho3DProject> project = MakeShared<Urho3DProject>(context_);
//     if (!project->LoadFile(fileName.toStdString().c_str()))
//     {
//         QMessageBox msgBox;
//         msgBox.setText("Failed to load project:\n" + fileName);
//         msgBox.setStandardButtons(QMessageBox::Ok);
//         msgBox.exec();
//         return false;
//     }
//     SetCurrentProject(project);
//     return true;
// }
// 
// void MainWindow::CloseProject()
// {
//     urho3DWidget_->SetCurrentProject(nullptr);
// }

void MainWindow1::AddPage(AbstractPage* page, bool makeActive)
{
    Q_ASSERT(page);

    // Add new tab
    page->setVisible(false);
    layout_->addWidget(page);
    pages_.Push(page);
    const int index = tabBar_->addTab(page->GetTitleDecorated());

    // Emit signal for first tab
    if (pages_.Size() == 1)
        OnPageChanged(tabBar_->currentIndex());

    // Activate
    if (makeActive)
        tabBar_->setCurrentIndex(index);

//     if (page)
//     {
//         const int index = tabWidget_->addTab(page, page->GetTitle());
//         connect(page, SIGNAL(titleChanged(AbstractPage*, const QString&)), this, SLOT(OnTabRenamed(AbstractPage*, const QString&)));
//         tabWidget_->setCurrentIndex(index);
//     }
}

void MainWindow1::ClosePage(AbstractPage* page)
{
    const int index = pages_.Find(page) - pages_.Begin();
    if (index < (int)pages_.Size())
    {
        tabBar_->removeTab(index);
        layout_->removeWidget(page);
        pages_.Remove(page);
    }
}

void MainWindow1::CloseAllPages()
{
    
}

void MainWindow1::UpdateMenu()
{
//     actionCloseProject_->setEnabled()
//     if (projectDocument_)
//     {
//     }
}

void MainWindow1::OnTabRenamed(AbstractPage* page, const QString& title)
{
//     for (int i = 0; i < tabWidget_->count(); ++i)
//         if (QWidget* tab = tabWidget_->widget(i))
//             if (tab == page)
//             {
//                 tabWidget_->setTabText(i, title);
//                 break;
//             }
}

void MainWindow1::OnPageChanged(int index)
{
    for (AbstractPage* page : pages_)
        page->setVisible(false);
    pages_[index]->setVisible(true);
    urho3DWidget_->setVisible(pages_[index]->IsUrho3DWidgetVisible());
//     if (AbstractDocument* document = dynamic_cast<AbstractDocument*>(tabWidget_->widget(index)))
//         ActivateDocument(document);
}

void MainWindow1::OnFileNewProject()
{
    Urho3DProjectPage* page = new Urho3DProjectPage(context_);
    if (page->Save(true))
        AddPage(page);
}

void MainWindow1::OnFileSave()
{
    if (AbstractPage* currentPage = GetCurrentPage())
        currentPage->Save(false);
}

void MainWindow1::OnFileOpenProject()
{
    Urho3DProjectPage page(context_);
    if (page.Open(true))
        SetCurrentProject(page.GetProject());
}

void MainWindow1::OnFileCloseProject()
{
    SetCurrentProject(nullptr);
}

void MainWindow1::OnFileNewScene()
{
}

void MainWindow1::OnFileOpenScene()
{
    using namespace Urho3D;
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    const String folder = cache->GetResourceDirs()[0];

    const QString sceneFilter = "XML Scene (*.xml);;JSON Scene (*.json);;Binary Scene (*.bin);;All files (*.*)";
    const QString fileName = QFileDialog::getOpenFileName(this, "Open Scene...", "", sceneFilter);
    QFileDialog d;
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);

        SharedPtr<Scene> scene(new Scene(context_));
        File file(context_);
        if (file.Open(fileInfo.absoluteFilePath().toStdString().c_str()))
        {
            scene->LoadXML(file);
        }
        
        SceneEditorPage* document = new SceneEditorPage(GetUrho3DWidget(), fileInfo.fileName());
        document->SetScene(scene);
    }
//     QFileDialog dialog;
//     dialog.setAcceptMode(QFileDialog::AcceptOpen);
//     dialog.setFileMode(QFileDialog::ExistingFile);
//     dialog.show();
//     const QStringList files = dialog.selectedFiles();
}

void MainWindow1::OnFileSaveScene()
{

}

void MainWindow1::OnFileExit()
{
    close();
}

}
