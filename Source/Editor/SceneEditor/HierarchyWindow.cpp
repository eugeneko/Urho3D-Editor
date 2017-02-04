#include "HierarchyWindow.h"
// #include "Configuration.h"
#include "SceneDocument.h"
#include "../MainWindow.h"
#include "../Bridge.h"
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/SceneEvents.h>
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

int ObjectHierarchyItem::FindChild(Urho3D::Object* object, int hintRow /*= -1*/) const
{
    // Try hint first
    if (hintRow >= 0 && hintRow < children_.size())
        if (children_[hintRow]->GetObject() == object)
            return hintRow;

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

Urho3D::Object* ObjectHierarchyModel::GetParentObject(Urho3D::Object* object)
{
    if (Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object))
        return node->GetParent();
    else if (Urho3D::Component* component = dynamic_cast<Urho3D::Component*>(object))
        return component->GetNode();
    else
        return nullptr;
}

int ObjectHierarchyModel::GetChildIndex(Urho3D::Object* object, Urho3D::Object* parent)
{
    using namespace Urho3D;
    if (Node* parentNode = dynamic_cast<Node*>(parent))
    {
        if (Component* component = dynamic_cast<Component*>(object))
        {
            const Vector<SharedPtr<Component>>& components = parentNode->GetComponents();
            for (unsigned i = 0; i < components.Size(); ++i)
                if (components[i] == component)
                    return (int)i;
        }
        else if (Node* node = dynamic_cast<Node*>(object))
        {
            const Vector<SharedPtr<Node>>& children = parentNode->GetChildren();
            for (unsigned i = 0; i < children.Size(); ++i)
                if (children[i] == node)
                    return (int)i + parentNode->GetNumComponents();
        }
    }
    return -1;
}

ObjectHierarchyItem* ObjectHierarchyModel::ConstructObjectItem(Urho3D::Object* object, ObjectHierarchyItem* parentItem)
{
    QScopedPointer<ObjectHierarchyItem> item(new ObjectHierarchyItem(parentItem));
    item->SetObject(object);

    if (Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object))
        ConstructNodeItem(item.data(), node);

    return item.take();
}

ObjectHierarchyModel::ObjectHierarchyModel()
    : rootItem_(new ObjectHierarchyItem(nullptr))
{
}

QModelIndex ObjectHierarchyModel::FindIndex(Urho3D::Object* object, QModelIndex hint /*= QModelIndex()*/)
{
    if (!object)
        return QModelIndex();
    if (hint.isValid() && GetObject(hint) == object)
        return hint;

    GetObjectHierarchy(object, tempHierarchy_);
    QModelIndex result;
    while (!tempHierarchy_.empty())
    {
        Urho3D::Object* child = tempHierarchy_.takeLast();
        const int row = GetItem(result)->FindChild(child);
        if (row < 0)
            return QModelIndex();
        result = index(row, 0, result);
        assert(GetItem(result)->GetObject() == child);
    }
    return result;
}

ObjectHierarchyItem* ObjectHierarchyModel::GetItem(const QModelIndex& index) const
{
    if (index.isValid())
    {
        ObjectHierarchyItem* item = static_cast<ObjectHierarchyItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem_.data();
}

Urho3D::Object* ObjectHierarchyModel::GetObject(const QModelIndex& index) const
{
    ObjectHierarchyItem* item = GetItem(index);
    return item ? item->GetObject() : nullptr;
}

void ObjectHierarchyModel::UpdateObject(Urho3D::Object* object, QModelIndex hint /*= QModelIndex()*/)
{
    Urho3D::Object* parentObject = GetParentObject(object);
    const QModelIndex parentIndex = FindIndex(parentObject, hint.parent());
    DoRemoveObject(parentIndex, object, hint.row());
    DoAddObject(parentIndex, object);
}

void ObjectHierarchyModel::RemoveObject(Urho3D::Object* object, QModelIndex hint /*= QModelIndex()*/)
{
    Urho3D::Object* parentObject = GetParentObject(object);
    const QModelIndex parentIndex = FindIndex(parentObject, hint.parent());
    DoRemoveObject(parentIndex, object, hint.row());
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

    ObjectHierarchyItem* item = GetItem(index);

    Qt::ItemFlags result = QAbstractItemModel::flags(index);
    if (!item->GetObject()->IsInstanceOf<Urho3D::Scene>())
    {
        result |= Qt::ItemIsDragEnabled;
        if (item->GetObject()->IsInstanceOf<Urho3D::Node>())
            result |= Qt::ItemIsDropEnabled;
    }
    else
        result |= Qt::ItemIsDropEnabled;

    return result;
}

bool ObjectHierarchyModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
    int row, int column, const QModelIndex& parent)
{
    return true;
}

void ObjectHierarchyModel::DoAddObject(QModelIndex parentIndex, Urho3D::Object* object)
{
    using namespace Urho3D;

    ObjectHierarchyItem* parentItem = GetItem(parentIndex);

    if (!parentIndex.isValid())
    {
        beginInsertRows(parentIndex, 0, 0);
        parentItem->InsertChild(0, ConstructObjectItem(object, parentItem));
        endInsertRows();
    }
    else
    {
        const int childIndex = GetChildIndex(object, parentItem->GetObject());
        if (childIndex >= 0)
        {
            beginInsertRows(parentIndex, childIndex, childIndex);
            parentItem->InsertChild(childIndex, ConstructObjectItem(object, parentItem));
            endInsertRows();
        }
    }
}

void ObjectHierarchyModel::DoRemoveObject(QModelIndex parentIndex, Urho3D::Object* object, int hintRow /*= -1*/)
{
    ObjectHierarchyItem* parentItem = GetItem(parentIndex);
    const int objectIndex = parentItem->FindChild(object, hintRow);
    if (objectIndex >= 0)
    {
        beginRemoveRows(parentIndex, objectIndex, objectIndex);
        parentItem->RemoveChild(objectIndex);
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
    : Object(document.GetContext())
    , document_(document)
    , layout_(new QGridLayout())
    , treeView_(new QTreeView())
    , treeModel_(new ObjectHierarchyModel())
    , suppressSceneSelectionChanged_(false)
{
    treeModel_->UpdateObject(&document.GetScene());

    treeView_->header()->hide();
    treeView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView_->setDragDropMode(QAbstractItemView::DragDrop);
    treeView_->setDragEnabled(true);
    treeView_->setModel(treeModel_.data());
    treeView_->setContextMenuPolicy(Qt::CustomContextMenu);

    layout_->addWidget(treeView_.data(), 0, 0);
    setLayout(layout_.data());

    connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(HandleTreeSelectionChanged()));
    connect(&document_, SIGNAL(selectionChanged()), this, SLOT(HandleSceneSelectionChanged()));
    connect(treeView_.data(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(HandleContextMenuRequested(const QPoint&)));

    Urho3D::Scene& scene = document_.GetScene();
    SubscribeToEvent(&scene, Urho3D::E_NODEADDED, URHO3D_HANDLER(HierarchyWindowWidget, HandleNodeAdded));
    SubscribeToEvent(&scene, Urho3D::E_NODEREMOVED, URHO3D_HANDLER(HierarchyWindowWidget, HandleNodeRemoved));
    SubscribeToEvent(&scene, Urho3D::E_COMPONENTADDED, URHO3D_HANDLER(HierarchyWindowWidget, HandleComponentAdded));
    SubscribeToEvent(&scene, Urho3D::E_COMPONENTREMOVED, URHO3D_HANDLER(HierarchyWindowWidget, HandleComponentRemoved));

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
        if (toUnselect.contains(treeModel_->GetObject(index)))
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

void HierarchyWindowWidget::HandleContextMenuRequested(const QPoint& point)
{
    const QModelIndex index = treeView_->indexAt(point);
    const QPoint globalPoint = treeView_->mapToGlobal(point);
    treeView_->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    if (Urho3D::Object* object = treeModel_->GetObject(index))
    {
        if (Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object))
        {
            if (QMenu* menu = document_.GetMainWindow().GetMenu("HierarchyWindow.Node"))
                menu->exec(globalPoint);
        }
        else if (Urho3D::Component* node = dynamic_cast<Urho3D::Component*>(object))
        {
            if (QMenu* menu = document_.GetMainWindow().GetMenu("HierarchyWindow.Component"))
                menu->exec(globalPoint);
        }
    }
}

void HierarchyWindowWidget::HandleNodeAdded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;
    Node* node = dynamic_cast<Node*>(eventData[NodeAdded::P_NODE].GetPtr());
    treeModel_->UpdateObject(node);
}

void HierarchyWindowWidget::HandleNodeRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;
    Node* node = dynamic_cast<Node*>(eventData[NodeRemoved::P_NODE].GetPtr());
    treeModel_->RemoveObject(node);
}

void HierarchyWindowWidget::HandleComponentAdded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;
    Component* component = dynamic_cast<Component*>(eventData[ComponentAdded::P_COMPONENT].GetPtr());
    treeModel_->UpdateObject(component);
}

void HierarchyWindowWidget::HandleComponentRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;
    Component* component = dynamic_cast<Component*>(eventData[ComponentRemoved::P_COMPONENT].GetPtr());
    treeModel_->RemoveObject(component);
}

QSet<Urho3D::Object*> HierarchyWindowWidget::GatherSelection()
{
    const QModelIndexList selectedIndexes = treeView_->selectionModel()->selectedIndexes();
    QSet<Urho3D::Object*> selectedObjects;
    for (const QModelIndex& selectedIndex : selectedIndexes)
    {
        if (Urho3D::Object* object = treeModel_->GetObject(selectedIndex))
            selectedObjects.insert(object);
    }
    return selectedObjects;
}

}
