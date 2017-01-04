#include "HierarchyWindow.h"
// #include "Configuration.h"
#include "MainWindow.h"
#include "SceneEditor.h"
#include "Bridge.h"
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHeaderView>

namespace Urho3DEditor
{

HierarchyWindow::HierarchyWindow()
{

}

bool HierarchyWindow::DoInitialize()
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

void HierarchyWindow::HandleViewHierarchyWindow(bool checked)
{
    if (checked)
    {
        hierarchyWindow_.reset(new HierarchyWindowWidget(*mainWindow_));
        mainWindow_->AddDock(Qt::LeftDockWidgetArea, hierarchyWindow_.data());
    }
    else
    {
        hierarchyWindow_->close();
        hierarchyWindow_.reset();
    }
}

//////////////////////////////////////////////////////////////////////////
ObjectHierarchyItem::ObjectHierarchyItem(ObjectHierarchyItem *parent /*= 0*/)
    : node_(nullptr)
    , component_(nullptr)
    , parent_(parent)
{
}

ObjectHierarchyItem::~ObjectHierarchyItem()
{
    qDeleteAll(children_);
}

void ObjectHierarchyItem::SetNode(Urho3D::Node* node)
{
    node_ = node;
}

void ObjectHierarchyItem::SetComponent(Urho3D::Component* component)
{
    component_ = component;
    node_ = component ? component->GetNode() : nullptr;
}

bool ObjectHierarchyItem::InsertChild(int position, ObjectHierarchyItem* item)
{
    if (position < 0 || position > children_.size())
        return false;

    children_.insert(position, item);
    return true;

}

bool ObjectHierarchyItem::RemoveChild(int position)
{
    if (position < 0 || position >= children_.size())
        return false;

    delete children_.takeAt(position);

    return true;
}

int ObjectHierarchyItem::FindChild(Urho3D::Node* node) const
{
    for (int i = 0; i < children_.size(); ++i)
        if (!children_[i]->GetComponent() && children_[i]->GetNode() == node)
            return i;
    return -1;
}

int ObjectHierarchyItem::FindChild(Urho3D::Component* component) const
{
    for (int i = 0; i < children_.size(); ++i)
        if (children_[i]->GetComponent() == component)
            return i;
    return -1;
}

QString ObjectHierarchyItem::GetText() const
{
    if (component_)
        return Cast(component_->GetTypeName());
    else if (node_)
        return Cast(node_->GetName().Empty() ? node_->GetTypeName() : node_->GetName());
    else
        return "!!! null !!!";
}

//////////////////////////////////////////////////////////////////////////
void ObjectHierarchyModel::GetNodeHierarchy(Urho3D::Node* node, QVector<Urho3D::Node*>& hierarchy)
{
    hierarchy.clear();
    do
    {
        hierarchy.push_back(node);
        node = node->GetParent();
    } while (node);
}

ObjectHierarchyModel::ObjectHierarchyModel()
    : rootItem_(new ObjectHierarchyItem(nullptr))
{
}

QModelIndex ObjectHierarchyModel::FindIndex(Urho3D::Node* node)
{
    if (!node)
        return QModelIndex();

    GetNodeHierarchy(node, tempHierarchy_);
    QModelIndex result;
    while (!tempHierarchy_.empty())
    {
        const int row = GetItem(result)->FindChild(tempHierarchy_.takeLast());
        if (row < 0)
            return QModelIndex();
        result = index(row, 0, result);
    }
    return result;
}

void ObjectHierarchyModel::UpdateComponent(Urho3D::Component* component)
{
    Urho3D::Node* node = component->GetNode();
    QModelIndex nodeIndex = FindIndex(node);
    if (nodeIndex.isValid())
    {
        DoRemoveComponent(nodeIndex, component);
        DoAddComponent(nodeIndex, component);
    }
}

void ObjectHierarchyModel::RemoveComponent(Urho3D::Component* component)
{
    Urho3D::Node* node = component->GetNode();
    QModelIndex nodeIndex = FindIndex(node);
    if (nodeIndex.isValid())
        DoRemoveComponent(nodeIndex, component);
}

void ObjectHierarchyModel::RemoveNode(Urho3D::Node* node)
{
    using namespace Urho3D;

    Urho3D::Node* parent = node->GetParent();
    QModelIndex parentIndex = FindIndex(parent);
    DoRemoveNode(parentIndex, node);
}

void ObjectHierarchyModel::UpdateNode(Urho3D::Node* node)
{
    Urho3D::Node* parent = node->GetParent();
    QModelIndex parentIndex = FindIndex(parent);
    DoRemoveNode(parentIndex, node);
    DoAddNode(parentIndex, node);
}

QVariant ObjectHierarchyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    ObjectHierarchyItem* item = GetItem(index);
    switch (role)
    {
    case Qt::DisplayRole:
        return item->GetText();
    case Qt::TextColorRole:
        return item->GetComponent() ? QColor(178, 255, 178) : QColor(255, 255, 255); // #TODO Make configurable
    default:
        return QVariant();
    }
}

QVariant ObjectHierarchyModel::headerData(int section, Qt::Orientation orientation,
    int role) const
{
    return QVariant();
}

QModelIndex ObjectHierarchyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    ObjectHierarchyItem* parentItem = GetItem(parent);

    ObjectHierarchyItem* childItem = parentItem->GetChild(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ObjectHierarchyModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ObjectHierarchyItem* childItem = GetItem(index);
    ObjectHierarchyItem* parentItem = childItem->GetParent();

    if (parentItem == rootItem_.data())
        return QModelIndex();

    return createIndex(parentItem->GetChildNumber(), 0, parentItem);
}

int ObjectHierarchyModel::rowCount(const QModelIndex &parent) const
{
    ObjectHierarchyItem* parentItem = GetItem(parent);

    return parentItem->GetChildCount();
}

Qt::ItemFlags ObjectHierarchyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return /*Qt::ItemIsEditable |*/ QAbstractItemModel::flags(index);
}

ObjectHierarchyItem* ObjectHierarchyModel::GetItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        ObjectHierarchyItem* item = static_cast<ObjectHierarchyItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem_.data();
}

void ObjectHierarchyModel::DoAddComponent(QModelIndex nodeIndex, Urho3D::Component* component)
{
    using namespace Urho3D;

    Node* node = component->GetNode();
    ObjectHierarchyItem* nodeItem = GetItem(nodeIndex);
    const Vector<SharedPtr<Component>>& components = node->GetComponents();
    for (unsigned i = 0; i < components.Size(); ++i)
    {
        if (components[i] == component)
        {
            const int componentRow = (int)i;
            ObjectHierarchyItem* componentItem = new ObjectHierarchyItem(nodeItem);
            componentItem->SetComponent(component);
            beginInsertRows(nodeIndex, componentRow, componentRow);
            nodeItem->InsertChild(componentRow, componentItem);
            endInsertRows();
            break;
        }
    }
}

void ObjectHierarchyModel::DoRemoveComponent(QModelIndex nodeIndex, Urho3D::Component* component)
{
    ObjectHierarchyItem* nodeItem = GetItem(nodeIndex);
    const int componentIndex = nodeItem->FindChild(component);
    if (componentIndex > 0)
    {
        beginRemoveRows(nodeIndex, componentIndex, componentIndex);
        nodeItem->RemoveChild(componentIndex);
        endRemoveRows();
    }
}

void ObjectHierarchyModel::DoAddNode(QModelIndex parentIndex, Urho3D::Node* node)
{
    using namespace Urho3D;

    ObjectHierarchyItem* parentItem = GetItem(parentIndex);

    if (!parentIndex.isValid())
    {
        ObjectHierarchyItem* nodeItem = new ObjectHierarchyItem(parentItem);
        nodeItem->SetNode(node);
        ConstructNodeItem(nodeItem, node);

        beginInsertRows(parentIndex, 0, 0);
        parentItem->InsertChild(0, nodeItem);
        endInsertRows();
    }
    else
    {
        Node* parentNode = parentItem->GetNode();
        const Vector<SharedPtr<Node>>& children = parentNode->GetChildren();
        for (unsigned i = 0; i < children.Size(); ++i)
        {
            if (children[i] == node)
            {
                const int nodeRow = (int)i;
                ObjectHierarchyItem* nodeItem = new ObjectHierarchyItem(parentItem);
                nodeItem->SetNode(node);
                ConstructNodeItem(nodeItem, node);

                beginInsertRows(parentIndex, nodeRow, nodeRow);
                parentItem->InsertChild(nodeRow, nodeItem);
                endInsertRows();
                break;
            }
        }
    }
}

void ObjectHierarchyModel::DoRemoveNode(QModelIndex parentIndex, Urho3D::Node* node)
{
    ObjectHierarchyItem* parentItem = GetItem(parentIndex);
    const int nodeIndex = parentItem->FindChild(node);
    if (nodeIndex > 0)
    {
        beginRemoveRows(parentIndex, nodeIndex, nodeIndex);
        parentItem->RemoveChild(nodeIndex);
        endRemoveRows();
    }
}

void ObjectHierarchyModel::ConstructNodeItem(ObjectHierarchyItem* item, Urho3D::Node* node)
{
    using namespace Urho3D;

    // Add components
    const Vector<SharedPtr<Component>>& components = node->GetComponents();
    for (unsigned i = 0; i < components.Size(); ++i)
    {
        ObjectHierarchyItem* componentItem = new ObjectHierarchyItem(item);
        componentItem->SetComponent(components[i]);
        item->AppendChild(componentItem);
    }

    // Add children
    const Vector<SharedPtr<Node>>& children = node->GetChildren();
    for (unsigned i = 0; i < children.Size(); ++i)
    {
        ObjectHierarchyItem* childItem = new ObjectHierarchyItem(item);
        childItem->SetNode(children[i]);
        item->AppendChild(childItem);

        ConstructNodeItem(childItem, children[i]);
    }
}

//////////////////////////////////////////////////////////////////////////
HierarchyWindowPageWidget::HierarchyWindowPageWidget(ScenePage* page)
    : page_(page)
    , layout_(new QGridLayout())
    , treeView_(new QTreeView())
    , treeModel_(new ObjectHierarchyModel())
{
    treeView_->header()->hide();
    treeView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView_->setDragDropMode(QAbstractItemView::DragDrop);
    treeView_->setDragEnabled(true);
    treeView_->setModel(treeModel_.data());

    layout_->addWidget(treeView_.data(), 0, 0);
    setLayout(layout_.data());

    connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(HandleSelectionChanged()));
}

void HierarchyWindowPageWidget::HandleSelectionChanged()
{

}

//////////////////////////////////////////////////////////////////////////
HierarchyWindowWidget::HierarchyWindowWidget(MainWindow& mainWindow)
    : QDockWidget("Hierarchy Window")
    , mainWindow_(mainWindow)
{
    connect(&mainWindow, SIGNAL(pageChanged(MainWindowPage*)), this, SLOT(HandleCurrentPageChanged(MainWindowPage*)));
    connect(&mainWindow, SIGNAL(pageClosed(MainWindowPage*)),  this, SLOT(HandlePageClosed(MainWindowPage*)));

    HandleCurrentPageChanged(mainWindow_.GetCurrentPage());
}

void HierarchyWindowWidget::CreateWidget(ScenePage* page)
{
    if (!pages_[page])
    {
        pages_[page] = QSharedPointer<HierarchyWindowPageWidget>(new HierarchyWindowPageWidget(page));
        pages_[page]->GetModel().UpdateNode(&page->GetScene());
    }
}

void HierarchyWindowWidget::HandleCurrentPageChanged(MainWindowPage* page)
{
    if (ScenePage* scenePage = dynamic_cast<ScenePage*>(page))
    {
        CreateWidget(scenePage);
        setWidget(pages_[scenePage].data());
    }
    else
    {
        setWidget(nullptr);
    }
}

void HierarchyWindowWidget::HandlePageClosed(MainWindowPage* page)
{
    if (ScenePage* scenePage = dynamic_cast<ScenePage*>(page))
        pages_.remove(scenePage);
}

}
