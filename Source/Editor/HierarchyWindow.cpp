#include "HierarchyWindow.h"
// #include "Configuration.h"
#include "MainWindow.h"
#include "SceneEditor.h"
#include "Bridge.h"
#include <QTreeView>
#include <QVBoxLayout>
#include <QHeaderView>

namespace Urho3DEditor
{

HierarchyWindowManager::HierarchyWindowManager()
{

}

bool HierarchyWindowManager::DoInitialize()
{
    mainWindow_ = GetModule<MainWindow>();
    if (!mainWindow_ /*|| !config_*/)
        return false;

    QMenu* menuView = mainWindow_->GetTopLevelMenu(MainWindow::MenuView);
    if (!menuView)
        return false;

    actionViewHierarchyWindow_.reset(menuView->addAction("Hierarchy Window"));
    actionViewHierarchyWindow_->setCheckable(true);
    connect(actionViewHierarchyWindow_.data(), SIGNAL(triggered(bool)), this, SLOT(HandleViewHierarchyWindow(bool)));

    actionViewHierarchyWindow_->activate(QAction::Trigger);
    return true;
}

void HierarchyWindowManager::HandleViewHierarchyWindow(bool checked)
{
    if (checked)
    {
        hierarchyWindow_.reset(new HierarchyWindow(*mainWindow_));
        mainWindow_->AddDock(Qt::LeftDockWidgetArea, hierarchyWindow_.data());
    }
    else
    {
        hierarchyWindow_->close();
        hierarchyWindow_.reset();
    }
}

//////////////////////////////////////////////////////////////////////////
HierarchyWindow::HierarchyWindow(MainWindow& mainWindow)
    : QDockWidget("Hierarchy Window")
    , mainWindow_(mainWindow)
    , layout_(new QVBoxLayout())
    , treeView_(new QTreeView())
{
    connect(&mainWindow, SIGNAL(pageChanged(MainWindowPage*)), this, SLOT(HandleCurrentPageChanged(MainWindowPage*)));
    connect(&mainWindow, SIGNAL(pageClosed(MainWindowPage*)),  this, SLOT(HandlePageClosed(MainWindowPage*)));

    treeView_->header()->hide();
    setWidget(treeView_.data());
    //layout_->addWidget(treeView_.data());
    //setLayout(layout_.data());
}

void HierarchyWindow::RebuildHierarchy(ScenePage* page, Urho3D::Node& root)
{
    QScopedPointer<TreeItem> rootItem(new TreeItem({ "" }));
    rootItem->appendChild(BuildTreeItems(root, rootItem.data()));
    QSharedPointer<TreeModel> model(new TreeModel(rootItem.take()));
    trees_[page] = model;
}

TreeItem* HierarchyWindow::BuildTreeItems(Urho3D::Node& node, TreeItem* parent) const
{
    using namespace Urho3D;

    QVector<QVariant> data;
    data << Cast(node.GetName().Empty() ? node.GetTypeName() : node.GetName());
    TreeItem* item = new TreeItem(data, parent);

    // Add components
    const Vector<SharedPtr<Component>>& components = node.GetComponents();
    for (unsigned i = 0; i < components.Size(); ++i)
    {
        QVector<QVariant> data;
        data << Cast(components[i]->GetTypeName());
        item->appendChild(new TreeItem(data, item));
    }

    // Add children
    const Vector<SharedPtr<Node>>& children = node.GetChildren();
    for (unsigned i = 0; i < children.Size(); ++i)
        item->appendChild(BuildTreeItems(*children[i], item));

    return item;
}

void HierarchyWindow::HandleCurrentPageChanged(MainWindowPage* page)
{
    if (ScenePage* scenePage = dynamic_cast<ScenePage*>(page))
    {
        if (!trees_[scenePage])
            RebuildHierarchy(scenePage, scenePage->GetScene());
        treeView_->setModel(trees_[scenePage].data());
    }
    else
    {
        treeView_->setModel(nullptr);
    }
}

void HierarchyWindow::HandlePageClosed(MainWindowPage* page)
{

}

}
