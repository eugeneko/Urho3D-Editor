#pragma once

#include <Urho3D/Core/Object.h>
#include <QAbstractItemModel>
#include <QMimeData>

class QTreeView;

namespace Urho3DEditor
{

/// Item of object hierarchy.
class ObjectHierarchyItem : public QObject
{
    Q_OBJECT

public:
    /// Construct.
    explicit ObjectHierarchyItem(ObjectHierarchyItem *parent = 0);
    /// Destruct.
    ~ObjectHierarchyItem();

    /// Set object.
    void SetObject(Urho3D::Object* object);

    /// Append child to item. Ownership is transferred to this item.
    void AppendChild(ObjectHierarchyItem* item) { children_.push_back(item); }
    /// Insert child.
    bool InsertChild(int position, ObjectHierarchyItem* item);
    /// Remove child.
    bool RemoveChild(int position);

    /// Find child with specified object. O(1) if hint is correct, O(num_children) otherwise.
    int FindChild(Urho3D::Object* object, int hintRow = -1) const;
    /// Get object.
    Urho3D::Object* GetObject() const { return object_; }

    /// Get child item.
    ObjectHierarchyItem* GetChild(int number) { return children_.value(number); }
    /// Get number of children.
    int GetChildCount() const { return children_.size(); }
    /// Get parent item.
    ObjectHierarchyItem* GetParent() const { return parent_; }
    /// Get number of this item.
    int GetChildNumber() { return parent_ ? parent_->children_.indexOf(this) : 0; }

private:
    /// Object.
    Urho3D::WeakPtr<Urho3D::Object> object_;
    /// Children.
    QList<ObjectHierarchyItem*> children_;
    /// Parent.
    ObjectHierarchyItem* parent_;

};

/// Contains specialization of operations that can't be performed over generic objects.
class ObjectHierarchySpecialization
{
public:
    /// Construct object item.
    virtual ObjectHierarchyItem* ConstructObjectItem(Urho3D::Object* object, ObjectHierarchyItem* parentItem) = 0;

    /// Get hierarchy of the object.
    virtual void GetObjectHierarchy(Urho3D::Object* object, QVector<Urho3D::Object*>& hierarchy) = 0;
    /// Get parent object.
    virtual Urho3D::Object* GetParentObject(Urho3D::Object* object) = 0;
    /// Get index of child object among parent children.
    virtual int GetChildIndex(Urho3D::Object* object, Urho3D::Object* parent) = 0;

    /// Get object name.
    virtual QString GetObjectName(Urho3D::Object* object) = 0;
    /// Get object text.
    virtual QString GetObjectText(Urho3D::Object* object) = 0;
    /// Get object color.
    virtual QColor GetObjectColor(Urho3D::Object* object) = 0;
    /// Return whether the object is drag-able.
    virtual bool IsDragable(Urho3D::Object* object) = 0;
    /// Return whether the object is drop-able.
    virtual bool IsDropable(Urho3D::Object* object) = 0;

    /// Construct mime data.
    virtual QMimeData* ConstructMimeData(const QModelIndexList& indexes) = 0;
    /// Check whether can drop mime data.
    virtual bool CanDropMime(const QMimeData* data, const QModelIndex& parent, int row) = 0;
    /// Drop mime data.
    virtual bool DropMime(const QMimeData* data, const QModelIndex& parent, int row) = 0;
};

/// Model of object hierarchy.
class ObjectHierarchyModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /// Construct.
    ObjectHierarchyModel(ObjectHierarchySpecialization& spec);
    /// Get index of object. O(1) if hint is correct, O(object_depth*average_num_children) otherwise.
    QModelIndex FindIndex(Urho3D::Object* object, QModelIndex hint = QModelIndex());
    /// Get item by index.
    ObjectHierarchyItem* GetItem(const QModelIndex& index) const;
    /// Get object by index.
    Urho3D::Object* GetObject(const QModelIndex& index) const;
    /// Get objects by index list.
    QVector<Urho3D::Object*> GetObjects(const QModelIndexList& indices) const;

    /// Update object.
    void UpdateObject(Urho3D::Object* object, QModelIndex hint = QModelIndex());
    /// Remove object.
    void RemoveObject(Urho3D::Object* object, QModelIndex hint = QModelIndex());

public:
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& index) const override;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override { return 1; }

    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList& indexes) const override;
    virtual bool canDropMimeData(const QMimeData* data, Qt::DropAction action,
        int row, int column, const QModelIndex& parent) const override;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action,
        int row, int column, const QModelIndex& parent) override;
    virtual Qt::DropActions supportedDropActions() const override { return Qt::MoveAction | Qt::LinkAction; }
    virtual Qt::DropActions supportedDragActions() const override { return Qt::MoveAction; }

private:
    /// Add object.
    void DoAddObject(QModelIndex parentIndex, Urho3D::Object* object);
    /// Remove object.
    void DoRemoveObject(QModelIndex parentIndex, Urho3D::Object* object, int hintRow = -1);

private:
    /// Specialization.
    ObjectHierarchySpecialization& spec_;
    /// Root item.
    QScopedPointer<ObjectHierarchyItem> rootItem_;
    /// Temporary storage of object hierarchy.
    QVector<Urho3D::Object*> tempHierarchy_;
    /// Whether to suppress all updates.
    bool suppressUpdates_;

};

}

