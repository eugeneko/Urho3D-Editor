#pragma once

#include "../Module.h"
#include "../Widgets/ObjectHierarchyModel.h"
#include <QDockWidget>
#include <QGridLayout>
#include <QMimeData>

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

/// Mime data of object hierarchy.
class ObjectHierarchyMime : public QMimeData
{
    Q_OBJECT

public:
    /// Objects: nodes.
    QVector<Urho3D::Node*> nodes_;
    /// Objects: components.
    QVector<Urho3D::Component*> components_;
};

/// Hierarchy Window Widget.
class HierarchyWindowWidget : public QWidget, public Urho3D::Object, private ObjectHierarchySpecialization
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
    /// Handle component re-ordered.
    void HandleComponentReordered(Urho3D::Component& component);

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

    /// @see ObjectHierarchySpecialization::ConstructObjectItem
    virtual ObjectHierarchyItem* ConstructObjectItem(Urho3D::Object* object, ObjectHierarchyItem* parentItem) override;
    /// @see ObjectHierarchySpecialization::GetObjectHierarchy
    virtual void GetObjectHierarchy(Urho3D::Object* object, QVector<Urho3D::Object*>& hierarchy) override;
    /// @see ObjectHierarchySpecialization::GetParentObject
    virtual Urho3D::Object* GetParentObject(Urho3D::Object* object) override;
    /// @see ObjectHierarchySpecialization::GetChildIndex
    virtual int GetChildIndex(Urho3D::Object* object, Urho3D::Object* parent) override;
    /// @see ObjectHierarchySpecialization::GetObjectName
    virtual QString GetObjectName(Urho3D::Object* object) override;
    /// @see ObjectHierarchySpecialization::GetObjectText
    virtual QString GetObjectText(Urho3D::Object* object) override;
    /// @see ObjectHierarchySpecialization::GetObjectColor
    virtual QColor GetObjectColor(Urho3D::Object* object) override;
    /// @see ObjectHierarchySpecialization::IsDragable
    virtual bool IsDragable(Urho3D::Object* object) override;
    /// @see ObjectHierarchySpecialization::IsDropable
    virtual bool IsDropable(Urho3D::Object* object) override;
    /// @see ObjectHierarchySpecialization::ConstructMimeData
    virtual QMimeData* ConstructMimeData(const QModelIndexList& indexes) override;
    /// @see ObjectHierarchySpecialization::CanDropMime
    virtual bool CanDropMime(const QMimeData* data, const QModelIndex& parent, int row) override;
    /// @see ObjectHierarchySpecialization::DropMime
    virtual bool DropMime(const QMimeData* data, const QModelIndex& parent, int row) override;


private:
    /// Document.
    SceneDocument& document_;
    /// Layout.
    QScopedPointer<QGridLayout> layout_;
    /// Tree view.
    QScopedPointer<QTreeView> treeView_;
    /// Tree model.
    QScopedPointer<ObjectHierarchyModel> treeModel_;
    /// Whether to suppress scene selection changed.
    bool suppressSceneSelectionChanged_;

};

}

