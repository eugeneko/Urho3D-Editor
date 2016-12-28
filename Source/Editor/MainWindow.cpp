#include "MainWindow.h"
#include "EditorDocument.h"

#include <Urho3D/IO/File.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <QFileDialog>
#include <QFileInfo>
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

    connect(fileMenu->addAction("New Scene"), SIGNAL(triggered(bool)), this, SLOT(OnFileNewScene(bool)));
    connect(fileMenu->addAction("Open Scene"), SIGNAL(triggered(bool)), this, SLOT(OnFileOpenScene(bool)));
    connect(fileMenu->addAction("Save Scene"), SIGNAL(triggered(bool)), this, SLOT(OnFileSaveScene(bool)));
    connect(fileMenu->addAction("Exit"), SIGNAL(triggered(bool)), this, SLOT(OnFileExit(bool)));

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

void MainWindow::AddDocument(EditorDocument* document)
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
    AddDocument(new SceneDocument(GetUrho3DWidget(), "New Scene"));
}

void MainWindow::OnFileOpenScene(bool)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    const String folder = cache->GetResourceDirs()[0];

    const QString sceneFilter = "XML Scene (*.xml);;JSON Scene (*.json);;Binary Scene (*.bin);;All files (*.*)";
    const QString fileName = QFileDialog::getOpenFileName(this, "Open Scene...", folder.CString(), sceneFilter);
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        SharedPtr<Scene> scene(new Scene(context_));
        File file(context_);
        if (file.Open(fileInfo.absoluteFilePath().toStdString().c_str()))
        {
            scene->LoadXML(file);
        }
        
        SceneDocument* document = new SceneDocument(GetUrho3DWidget(), fileInfo.fileName());
        document->SetScene(scene);
        AddDocument(document);
    }
//     QFileDialog dialog;
//     dialog.setAcceptMode(QFileDialog::AcceptOpen);
//     dialog.setFileMode(QFileDialog::ExistingFile);
//     dialog.show();
//     const QStringList files = dialog.selectedFiles();
}

void MainWindow::OnFileSaveScene(bool)
{

}

void MainWindow::OnFileExit(bool)
{
    close();
}

}
