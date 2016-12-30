#include "MainWindow.h"
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

namespace Urho3D
{

//////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(Context* context)
    : Object(context)
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

MainWindow::~MainWindow()
{
}

void MainWindow::SetCurrentProject(SharedPtr<Urho3DProject> project)
{
    urho3DProject_ = project;
    urho3DWidget_->SetCurrentProject(urho3DProject_);
    static const QString title = "Urho3D Editor";
    if (urho3DProject_)
        setWindowTitle(title + ": " + urho3DProject_->GetName().CString());
    else
        setWindowTitle(title + ": No current project");
}

AbstractPage* MainWindow::GetCurrentPage() const
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

void MainWindow::AddPage(AbstractPage* page, bool makeActive)
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

void MainWindow::ClosePage(AbstractPage* page)
{
    const int index = pages_.Find(page) - pages_.Begin();
    if (index < (int)pages_.Size())
    {
        tabBar_->removeTab(index);
        layout_->removeWidget(page);
        pages_.Remove(page);
    }
}

void MainWindow::CloseAllPages()
{
    
}

void MainWindow::UpdateMenu()
{
//     actionCloseProject_->setEnabled()
//     if (projectDocument_)
//     {
//     }
}

void MainWindow::OnTabRenamed(AbstractPage* page, const QString& title)
{
//     for (int i = 0; i < tabWidget_->count(); ++i)
//         if (QWidget* tab = tabWidget_->widget(i))
//             if (tab == page)
//             {
//                 tabWidget_->setTabText(i, title);
//                 break;
//             }
}

void MainWindow::OnPageChanged(int index)
{
    for (AbstractPage* page : pages_)
        page->setVisible(false);
    pages_[index]->setVisible(true);
    urho3DWidget_->setVisible(pages_[index]->IsUrho3DWidgetVisible());
//     if (AbstractDocument* document = dynamic_cast<AbstractDocument*>(tabWidget_->widget(index)))
//         ActivateDocument(document);
}

void MainWindow::OnFileNewProject()
{
    Urho3DProjectPage* page = new Urho3DProjectPage(context_);
    if (page->Save(true))
        AddPage(page);
}

void MainWindow::OnFileSave()
{
    if (AbstractPage* currentPage = GetCurrentPage())
        currentPage->Save(false);
}

void MainWindow::OnFileOpenProject()
{
    Urho3DProjectPage page(context_);
    if (page.Open(true))
        SetCurrentProject(page.GetProject());
}

void MainWindow::OnFileCloseProject()
{
    SetCurrentProject(nullptr);
}

void MainWindow::OnFileNewScene()
{
}

void MainWindow::OnFileOpenScene()
{
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

void MainWindow::OnFileSaveScene()
{

}

void MainWindow::OnFileExit()
{
    close();
}

}
