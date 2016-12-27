#include "MainWindow.h"

#include <QTabBar>
#include <QPushButton>
#include <QHBoxLayout>

namespace Urho3D
{

MainWindow::MainWindow(Context* context)
    : Object(context)
    , clientWidget_(nullptr)
{

}

MainWindow::~MainWindow()
{

}

void MainWindow::CreateWidgets()
{
    // Create menu
    QMenu* fileMenu = menuBar()->addMenu("File");
    QAction* fileExitMenu = fileMenu->addAction("Exit");
    menuBar()->show();

    // Create window body
    QWidget* windowBody = new QWidget();

    QVBoxLayout* windowBodyLayout = new QVBoxLayout(this);

    QTabBar* documentTabBar = new QTabBar();
    documentTabBar->addTab("Scene1.xml");
    documentTabBar->addTab("Scene2.xml");
    windowBodyLayout->addWidget(documentTabBar);

    clientWidget_ = new QWidget();
    windowBodyLayout->addWidget(clientWidget_);

    windowBody->setLayout(windowBodyLayout);
    setCentralWidget(windowBody);
}

void MainWindow::InitializeEngine(QWidget* host)
{

}

}
