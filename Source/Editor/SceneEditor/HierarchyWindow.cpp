#include "HierarchyWindow.h"
// #include "Configuration.h"
#include "SceneDocument.h"
#include "../MainWindow.h"
#include "../Bridge.h"
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHeaderView>

namespace Urho3DEditor
{

bool HierarchyWindow::Initialize()
{
    MainWindow& mainWindow = GetMainWindow();

    // Connect to signals
    connect(&mainWindow, SIGNAL(currentDocumentChanged(Document*)), this, SLOT(HandleCurrentDocumentChanged(Document*)));
    connect(&mainWindow, SIGNAL(updateMenu(QMenu*)), this, SLOT(UpdateMenu()));

    // Create widget
    widget_.reset(new QDockWidget("Hierarchy Window"));
    widget_->hide();
    mainWindow.AddDock(Qt::LeftDockWidgetArea, widget_.data());

    // Create actions
    showAction_.reset(mainWindow.AddAction("View.HierarchyWindow"));
    showAction_->setCheckable(true);
    connect(showAction_.data(), SIGNAL(triggered(bool)), this, SLOT(ToggleShow(bool)));

    // Launch
    showAction_->activate(QAction::Trigger);
    CreateBody(mainWindow.GetCurrentDocument());
    return true;
}

void HierarchyWindow::ToggleShow(bool checked)
{
    widget_->setVisible(checked);
}

void HierarchyWindow::UpdateMenu()
{
    showAction_->setChecked(widget_->isVisible());
}

void HierarchyWindow::HandleCurrentDocumentChanged(Document* document)
{
    CreateBody(document);
}

void HierarchyWindow::CreateBody(Document* document)
{
    if (!widget_)
        return;

    HierarchyWindowWidget* bodyWidget = document->Get<HierarchyWindowWidget, SceneDocument>(widget_.data());
    if (bodyWidget)
        widget_->setWidget(bodyWidget);
    else
        widget_->setWidget(new QTreeView(widget_.data()));
}

void HierarchyWindow::HandleDockClosed()
{
    widget_.reset();
}

//////////////////////////////////////////////////////////////////////////
ObjectHierarchyItem::ObjectHierarchyItem(ObjectHierarchyItem *parent /*= 0*/)
    : parent_(parent)
{
}

ObjectHierarchyItem::~ObjectHierarchyItem()
{
    qDeleteAll(children_);
}

void ObjectHierarchyItem::SetObject(Urho3D::Object* object)
{
    object_ = object;
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

int ObjectHierarchyItem::FindChild(Urho3D::Object* object) const
{
    for (int i = 0; i < children_.size(); ++i)
        if (children_[i]->GetObject() == object)
            return i;
    return -1;
}

QString ObjectHierarchyItem::GetText() const
{
    if (Urho3D::Component* component = dynamic_cast<Urho3D::Component*>(object_.Get()))
        return Cast(component->GetTypeName());
    else if (Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object_.Get()))
        return Cast(node->GetName().Empty() ? node->GetTypeName() : node->GetName());
    else
        return "";
}

QColor ObjectHierarchyItem::GetColor() const
{
    if (Urho3D::Component* component = dynamic_cast<Urho3D::Component*>(object_.Get()))
        return QColor(178, 255, 178);
    else if (Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object_.Get()))
        return QColor(255, 255, 255);
    else
        return QColor(Qt::black);
    // #TODO Make configurable
}

//////////////////////////////////////////////////////////////////////////
void ObjectHierarchyModel::GetObjectHierarchy(Urho3D::Object* object, QVector<Urho3D::Object*>& hierarchy)
{
    hierarchy.clear();

    // Get child-most node
    Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object);
    if (!node)
    {
        Urho3D::Component* component = dynamic_cast<Urho3D::Component*>(object);
        if (!component)
            return;
        hierarchy.push_back(component);
        node = component->GetNode();
    }

    // Go to root
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

QModelIndex ObjectHierarchyModel::FindIndex(Urho3D::Object* object)
{
    if (!object)
        return QModelIndex();

    GetObjectHierarchy(object, tempHierarchy_);
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
        return item->GetColor();
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

    return /*Qt::ItemIsEditable |*/ Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);
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
            componentItem->SetObject(component);
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
        nodeItem->SetObject(node);
        ConstructNodeItem(nodeItem, node);

        beginInsertRows(parentIndex, 0, 0);
        parentItem->InsertChild(0, nodeItem);
        endInsertRows();
    }
    else
    {
        Node* parentNode = dynamic_cast<Node*>(parentItem->GetObject());
        const Vector<SharedPtr<Node>>& children = parentNode->GetChildren();
        for (unsigned i = 0; i < children.Size(); ++i)
        {
            if (children[i] == node)
            {
                const int nodeRow = (int)i;
                ObjectHierarchyItem* nodeItem = new ObjectHierarchyItem(parentItem);
                nodeItem->SetObject(node);
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
        componentItem->SetObject(components[i]);
        item->AppendChild(componentItem);
    }

    // Add children
    const Vector<SharedPtr<Node>>& children = node->GetChildren();
    for (unsigned i = 0; i < children.Size(); ++i)
    {
        ObjectHierarchyItem* childItem = new ObjectHierarchyItem(item);
        childItem->SetObject(children[i]);
        item->AppendChild(childItem);

        ConstructNodeItem(childItem, children[i]);
    }
}

//////////////////////////////////////////////////////////////////////////
HierarchyWindowWidget::HierarchyWindowWidget(SceneDocument& document)
    : document_(document)
    , layout_(new QGridLayout())
    , treeView_(new QTreeView())
    , treeModel_(new ObjectHierarchyModel())
    , suppressSceneSelectionChanged_(false)
{
    treeModel_->UpdateNode(&document.GetScene());

    treeView_->header()->hide();
    treeView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView_->setDragDropMode(QAbstractItemView::DragDrop);
    treeView_->setDragEnabled(true);
    treeView_->setModel(treeModel_.data());

    layout_->addWidget(treeView_.data(), 0, 0);
    setLayout(layout_.data());

    connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(HandleTreeSelectionChanged()));
    connect(&document_, SIGNAL(selectionChanged()), this, SLOT(HandleSceneSelectionChanged()));
}

HierarchyWindowWidget::~HierarchyWindowWidget()
{

}

void HierarchyWindowWidget::HandleTreeSelectionChanged()
{
    suppressSceneSelectionChanged_ = true;
    document_.SetSelection(GatherSelection());
    suppressSceneSelectionChanged_ = false;
}

void HierarchyWindowWidget::HandleSceneSelectionChanged()
{
    if (suppressSceneSelectionChanged_)
        return;
    QItemSelectionModel* selectionModel = treeView_->selectionModel();

    using Selection = QSet<Urho3D::Object*>;
    const Selection oldSelection = GatherSelection();
    const Selection& newSelection = document_.GetSelected();
    const Selection toUnselect = oldSelection - newSelection;
    const Selection toSelect = newSelection - oldSelection;

    // Deselect nodes
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
    for (QModelIndex index : selectedIndexes)
    {
        if (toUnselect.contains(treeModel_->GetItem(index)->GetObject()))
            selectionModel->select(index, QItemSelectionModel::Deselect);
    }

    // Select nodes
    bool wasScrolled = false;
    for (Urho3D::Object* object : toSelect)
    {
        QModelIndex index = treeModel_->FindIndex(object);
        selectionModel->select(index, QItemSelectionModel::Select);
        if (!wasScrolled)
        {
            wasScrolled = true;
            treeView_->scrollTo(index);
        }
    }
}

QSet<Urho3D::Object*> HierarchyWindowWidget::GatherSelection()
{
    const QModelIndexList selectedIndexes = treeView_->selectionModel()->selectedIndexes();
    QSet<Urho3D::Object*> selectedObjects;
    for (const QModelIndex& selectedIndex : selectedIndexes)
    {
        ObjectHierarchyItem* item = treeModel_->GetItem(selectedIndex);
        if (Urho3D::Object* object = item->GetObject())
            selectedObjects.insert(object);
    }
    return selectedObjects;
}

}
