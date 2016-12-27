#include "MainWindow.h"
#include "EditorDocument.h"

#include <QTabBar>

namespace Urho3D
{

MainWindow::MainWindow(Context* context)
    : Object(context)
    , urho3DWidget_(new Urho3DWidget(context_))
    , tabWidget_(new QTabWidget(this))
    , activeDocument_(nullptr)
{
    // Create menu
    QMenu* fileMenu = menuBar()->addMenu("File");
    QAction* fileNewSceneMenu = fileMenu->addAction("New Scene");
    QAction* fileExitMenu = fileMenu->addAction("Exit");

    connect(fileNewSceneMenu, SIGNAL(triggered(bool)), this, SLOT(OnFileNewScene(bool)));
    connect(fileExitMenu, SIGNAL(triggered(bool)), this, SLOT(OnFileExit(bool)));

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


    setCentralWidget(tabWidget_);
//     tabWidget_->tabBar()->setTabsClosable(true);
    connect(tabWidget_, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)));
//     tabWidget_->addTab(urho3DWidget_, "Scene1.xml");
//     tabWidget_->addTab(new QPushButton("Push"), "Scene2.xml");
}

MainWindow::~MainWindow()
{
    ActivateDocument(nullptr);
}

void MainWindow::AddDocument(EditorDocument* document, bool bringToTop)
{
    tabWidget_->addTab(document, document->GetName());
    //tabBar_->addTab("New tab");
    //tabBar_->set
}

void MainWindow::ActivateDocument(EditorDocument* document)
{
    if (activeDocument_)
        activeDocument_->Deactivate();
    activeDocument_ = document;
    if (activeDocument_)
        activeDocument_->Activate();
}

void MainWindow::OnTabChanged(int index)
{
    if (EditorDocument* document = dynamic_cast<EditorDocument*>(tabWidget_->widget(index)))
        ActivateDocument(document);
}

void MainWindow::OnFileNewScene(bool)
{
    AddDocument(new SceneDocument(GetUrho3DWidget(), "New Scene"), true);
}

void MainWindow::OnFileExit(bool)
{
    close();
}

}
