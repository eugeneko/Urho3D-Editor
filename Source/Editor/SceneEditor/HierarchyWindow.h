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

private:
    /// Initialize module.
    virtual bool Initialize() override;

private slots:
    /// Toggle show/hide.
    void ToggleShow(bool checked);
    /// Update menu.
    void UpdateMenu();
    /// Handle current document changed.
    void HandleCurrentDocumentChanged(Document* document);
    /// Handle dock closed.
    void HandleDockClosed();

private:
    /// Create body of the widget.
    void CreateBody(Document* document);

private:
    /// Show action.
    QScopedPointer<QAction> showAction_;

    /// Main dock widget.
    QScopedPointer<QDockWidget> widget_;

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

private:
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
    /// Get hierarchy of the object.
    static void GetObjectHierarchy(Urho3D::Object* object, QVector<Urho3D::Object*>& hierarchy);

    /// Construct.
    ObjectHierarchyModel();
    /// Get index of object.
    QModelIndex FindIndex(Urho3D::Object* object);
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

private:
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

private:
    /// Root item.
    QScopedPointer<ObjectHierarchyItem> rootItem_;
    /// Temporary storage of object hierarchy.
    QVector<Urho3D::Object*> tempHierarchy_;

};

/// Hierarchy Window Widget.
class HierarchyWindowWidget : public QWidget, public Urho3D::Object
{
    Q_OBJECT
    URHO3D_OBJECT(HierarchyWindowWidget, Urho3D::Object);

public:
    /// Construct.
    HierarchyWindowWidget(SceneDocument& document);
    /// Destruct.
    virtual ~HierarchyWindowWidget();
    /// Get model.
    ObjectHierarchyModel& GetModel() { return *treeModel_; }

private slots:
    /// Handle tree selection change.
    void HandleTreeSelectionChanged();
    /// Handle scene selection change.
    void HandleSceneSelectionChanged();
    /// Handle context menu request.
    void HandleContextMenuRequested(const QPoint& point);

private:
    /// Handle node added.
    void HandleNodeAdded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle node removed.
    void HandleNodeRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle component added.
    void HandleComponentAdded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle component removed.
    void HandleComponentRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);


private:
    /// Gather selected objects.
    QSet<Urho3D::Object*> GatherSelection();

private:
    /// Document.
    SceneDocument& document_;
    /// Layout.
    QScopedPointer<QGridLayout> layout_;
    /// Tree view.
    QScopedPointer<QTreeView> treeView_;
    /// Tree model.
    QScopedPointer<ObjectHierarchyModel> treeModel_;
    /// Set to suppress scene selection changed.
    bool suppressSceneSelectionChanged_;

};

}

