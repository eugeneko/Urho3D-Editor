#pragma once

#include "../Module.h"
#include <QDockWidget>
#include <QGridLayout>
#include <QAbstractItemModel>

class QTreeView;
class QVBoxLayout;

namespace Urho3D
{

class Node;
class Component;

}

namespace Urho3DEditor
{

class Configuration;
class MainWindow;
class Document;
class HierarchyWindowWidget;
class SceneDocument;

/// Hierarchy Window module.
class HierarchyWindow : public Module
{
    Q_OBJECT

public:
    /// Construct.
    HierarchyWindow();

protected:
    /// Initialize module.
    virtual bool Initialize() override;

protected slots:
    /// Handle 'View/Hierarchy Window'
    virtual void HandleViewHierarchyWindow(bool checked);
    /// Handle 'View/Hierarchy Window' is about to show.
    virtual void HandleViewHierarchyWindowAboutToShow();
    /// Handle current document changed.
    virtual void HandleCurrentPageChanged(Document* document);

private:
    /// 'View/Hierarchy Window' action.
    QScopedPointer<QAction> actionViewHierarchyWindow_;

    /// Hierarchy Window.
    QScopedPointer<QDockWidget> hierarchyWindow_;

};

/// Item of object hierarchy.
class ObjectHierarchyItem
{
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

    /// Find child with specified object.
    int FindChild(Urho3D::Object* object) const;
    /// Get object.
    Urho3D::Object* GetObject() const { return object_; }
    /// Get name of item.
    virtual QString GetText() const;
    /// Get color of item.
    virtual QColor GetColor() const;

    /// Get child item.
    ObjectHierarchyItem* GetChild(int number) { return children_.value(number); }
    /// Get number of children.
    int GetChildCount() const { return children_.size(); }
    /// Get parent item.
    ObjectHierarchyItem* GetParent() const { return parent_; }
    /// Get number of this item.
    int GetChildNumber() { return parent_ ? parent_->children_.indexOf(this) : 0; }

protected:
    /// Object.
    Urho3D::WeakPtr<Urho3D::Object> object_;
    /// Children.
    QList<ObjectHierarchyItem*> children_;
    /// Parent.
    ObjectHierarchyItem* parent_;

};

/// Model of object hierarchy.
class ObjectHierarchyModel : public QAbstractItemModel
{
public:
    /// Get node hierarchy. Note is the first, the root is the last.
    static void GetNodeHierarchy(Urho3D::Node* node, QVector<Urho3D::Node*>& hierarchy);

    /// Construct.
    ObjectHierarchyModel();
    /// Get index of node.
    QModelIndex FindIndex(Urho3D::Node* node);
    /// Get item by index.
    ObjectHierarchyItem *GetItem(const QModelIndex &index) const;

    /// Add or update component.
    void UpdateComponent(Urho3D::Component* component);
    /// Remove component.
    void RemoveComponent(Urho3D::Component* component);
    /// Add or update node.
    void UpdateNode(Urho3D::Node* node);
    /// Remove node.
    void RemoveNode(Urho3D::Node* node);

public:
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override { return 1; }

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    /// Add component.
    void DoAddComponent(QModelIndex nodeIndex, Urho3D::Component* component);
    /// Remove component.
    void DoRemoveComponent(QModelIndex nodeIndex, Urho3D::Component* component);
    /// Add node.
    void DoAddNode(QModelIndex parentIndex, Urho3D::Node* node);
    /// Remove node.
    void DoRemoveNode(QModelIndex parentIndex, Urho3D::Node* node);
    /// Construct node item.
    void ConstructNodeItem(ObjectHierarchyItem* item, Urho3D::Node* node);

protected:
    /// Root item.
    QScopedPointer<ObjectHierarchyItem> rootItem_;
    /// Temporary storage of node hierarchy.
    QVector<Urho3D::Node*> tempHierarchy_;

};

/// Hierarchy Window Widget.
class HierarchyWindowWidget : public QWidget
{
    Q_OBJECT

public:
    /// Construct.
    HierarchyWindowWidget(SceneDocument& document);
    /// Destruct.
    virtual ~HierarchyWindowWidget();
    /// Get model.
    ObjectHierarchyModel& GetModel() { return *treeModel_; }

protected slots:
    /// Handle selection change.
    virtual void HandleSelectionChanged();

protected:
    /// Document.
    SceneDocument& document_;
    /// Layout.
    QScopedPointer<QGridLayout> layout_;
    /// Tree view.
    QScopedPointer<QTreeView> treeView_;
    /// Tree model.
    QScopedPointer<ObjectHierarchyModel> treeModel_;
};

}

