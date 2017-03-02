#include "ObjectHierarchyModel.h"
#include "../Core/QtUrhoHelpers.h"
#include <QTreeView>

namespace Urho3DEditor
{

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

//////////////////////////////////////////////////////////////////////////
ObjectHierarchyModel::ObjectHierarchyModel(ObjectHierarchySpecialization& spec)
    : spec_(spec)
    , rootItem_(new ObjectHierarchyItem(nullptr))
    , suppressUpdates_(false)
{
}

QModelIndex ObjectHierarchyModel::FindIndex(Urho3D::Object* object, QModelIndex hint /*= QModelIndex()*/)
{
    if (!object)
        return QModelIndex();
    if (hint.isValid() && GetObject(hint) == object)
        return hint;

    spec_.GetObjectHierarchy(object, tempHierarchy_);
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

QVector<Urho3D::Object*> ObjectHierarchyModel::GetObjects(const QModelIndexList& indices) const
{
    QVector<Urho3D::Object*> objects;
    objects.reserve(indices.size());
    for (const QModelIndex& index : indices)
        objects.push_back(GetObject(index));
    return objects;
}

void ObjectHierarchyModel::UpdateObject(Urho3D::Object* object, QModelIndex hint /*= QModelIndex()*/)
{
    if (suppressUpdates_)
        return;
    Urho3D::Object* parentObject = spec_.GetParentObject(object);
    const QModelIndex parentIndex = FindIndex(parentObject, hint.parent());
    DoRemoveObject(parentIndex, object, hint.row());
    DoAddObject(parentIndex, object);
}

void ObjectHierarchyModel::RemoveObject(Urho3D::Object* object, QModelIndex hint /*= QModelIndex()*/)
{
    if (suppressUpdates_)
        return;
    Urho3D::Object* parentObject = spec_.GetParentObject(object);
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
        return spec_.GetObjectText(item->GetObject());
    case Qt::TextColorRole:
        return spec_.GetObjectColor(item->GetObject());
    default:
        return QVariant();
    }
}

QVariant ObjectHierarchyModel::headerData(int section, Qt::Orientation orientation, int role) const
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

    Urho3D::Object* object = GetObject(index);

    Qt::ItemFlags result = QAbstractItemModel::flags(index);
    if (spec_.IsDragable(object))
        result |= Qt::ItemIsDragEnabled;
    if (spec_.IsDropable(object))
        result |= Qt::ItemIsDropEnabled;

    return result;
}

QStringList ObjectHierarchyModel::mimeTypes() const
{
    return QStringList("text/plain");
}

QMimeData* ObjectHierarchyModel::mimeData(const QModelIndexList& indexes) const
{
    return spec_.ConstructMimeData(indexes);
}

bool ObjectHierarchyModel::canDropMimeData(const QMimeData* data, Qt::DropAction action,
    int row, int column, const QModelIndex& parent) const
{
    return spec_.CanDropMime(data, parent, row);
}

bool ObjectHierarchyModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
    int row, int column, const QModelIndex& parent)
{
    return spec_.DropMime(data, parent, row);
}

void ObjectHierarchyModel::DoAddObject(QModelIndex parentIndex, Urho3D::Object* object)
{
    using namespace Urho3D;

    ObjectHierarchyItem* parentItem = GetItem(parentIndex);

    if (!parentIndex.isValid())
    {
        beginInsertRows(parentIndex, 0, 0);
        parentItem->InsertChild(0, spec_.ConstructObjectItem(object, parentItem));
        endInsertRows();
    }
    else
    {
        const int childIndex = spec_.GetChildIndex(object, parentItem->GetObject());
        if (childIndex >= 0)
        {
            beginInsertRows(parentIndex, childIndex, childIndex);
            parentItem->InsertChild(childIndex, spec_.ConstructObjectItem(object, parentItem));
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

}
