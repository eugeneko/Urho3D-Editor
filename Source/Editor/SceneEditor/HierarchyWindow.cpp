#include "HierarchyWindow.h"
// #include "Configuration.h"
#include "SceneDocument.h"
#include "SceneActions.h"
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

namespace
{

/// Construct node item.
void ConstructNodeItem(ObjectHierarchyItem* item, Urho3D::Node* node)
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

}

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

    SceneHierarchyWidget* bodyWidget = document->Get<SceneHierarchyWidget, SceneDocument>(widget_.data());
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
SceneHierarchyWidget::SceneHierarchyWidget(SceneDocument& document)
    : Object(document.GetContext())
    , document_(document)
    , layout_(new QGridLayout())
    , treeView_(new QTreeView())
    , treeModel_(new ObjectHierarchyModel(*this))
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
    SubscribeToEvent(&scene, Urho3D::E_NODEADDED, URHO3D_HANDLER(SceneHierarchyWidget, HandleNodeAdded));
    SubscribeToEvent(&scene, Urho3D::E_NODEREMOVED, URHO3D_HANDLER(SceneHierarchyWidget, HandleNodeRemoved));
    SubscribeToEvent(&scene, Urho3D::E_COMPONENTADDED, URHO3D_HANDLER(SceneHierarchyWidget, HandleComponentAdded));
    SubscribeToEvent(&scene, Urho3D::E_COMPONENTREMOVED, URHO3D_HANDLER(SceneHierarchyWidget, HandleComponentRemoved));

}

SceneHierarchyWidget::~SceneHierarchyWidget()
{

}

void SceneHierarchyWidget::HandleTreeSelectionChanged()
{
    suppressSceneSelectionChanged_ = true;
    document_.SetSelection(GatherSelection());
    suppressSceneSelectionChanged_ = false;
}

void SceneHierarchyWidget::HandleSceneSelectionChanged()
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

void SceneHierarchyWidget::HandleContextMenuRequested(const QPoint& point)
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

void SceneHierarchyWidget::HandleNodeAdded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;
    Node* node = dynamic_cast<Node*>(eventData[NodeAdded::P_NODE].GetPtr());
    treeModel_->UpdateObject(node);
}

void SceneHierarchyWidget::HandleNodeRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;
    Node* node = dynamic_cast<Node*>(eventData[NodeRemoved::P_NODE].GetPtr());
    treeModel_->RemoveObject(node);
}

void SceneHierarchyWidget::HandleComponentAdded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;
    Component* component = dynamic_cast<Component*>(eventData[ComponentAdded::P_COMPONENT].GetPtr());
    treeModel_->UpdateObject(component);
}

void SceneHierarchyWidget::HandleComponentRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D;
    Component* component = dynamic_cast<Component*>(eventData[ComponentRemoved::P_COMPONENT].GetPtr());
    treeModel_->RemoveObject(component);
}

void SceneHierarchyWidget::HandleComponentReordered(Urho3D::Component& component)
{
    treeModel_->UpdateObject(&component);
}

QSet<Urho3D::Object*> SceneHierarchyWidget::GatherSelection()
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

//////////////////////////////////////////////////////////////////////////
Urho3DEditor::ObjectHierarchyItem* SceneHierarchyWidget::ConstructObjectItem(Urho3D::Object* object, ObjectHierarchyItem* parentItem)
{
    QScopedPointer<ObjectHierarchyItem> item(new ObjectHierarchyItem(parentItem));
    item->SetObject(object);

    if (Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object))
        ConstructNodeItem(item.data(), node);

    return item.take();
}

void SceneHierarchyWidget::GetObjectHierarchy(Urho3D::Object* object, QVector<Urho3D::Object*>& hierarchy)
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

Urho3D::Object* SceneHierarchyWidget::GetParentObject(Urho3D::Object* object)
{
    if (Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object))
        return node->GetParent();
    else if (Urho3D::Component* component = dynamic_cast<Urho3D::Component*>(object))
        return component->GetNode();
    else
        return nullptr;
}

int SceneHierarchyWidget::GetChildIndex(Urho3D::Object* object, Urho3D::Object* parent)
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

QString SceneHierarchyWidget::GetObjectName(Urho3D::Object* object)
{
    if (Urho3D::Component* component = dynamic_cast<Urho3D::Component*>(object))
        return Cast(component->GetTypeName());
    else if (Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object))
        return Cast(node->GetName().Empty() ? node->GetTypeName() : node->GetName());
    else
        return "";
}

QString SceneHierarchyWidget::GetObjectText(Urho3D::Object* object)
{
    return GetObjectName(object);
}

QColor SceneHierarchyWidget::GetObjectColor(Urho3D::Object* object)
{
    if (Urho3D::Component* component = dynamic_cast<Urho3D::Component*>(object))
        return QColor(178, 255, 178);
    else if (Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object))
        return QColor(255, 255, 255);
    else
        return QColor(Qt::black);
    // #TODO Make configurable
}

bool SceneHierarchyWidget::IsDragable(Urho3D::Object* object)
{
    return !object->IsInstanceOf<Urho3D::Scene>();
}

bool SceneHierarchyWidget::IsDropable(Urho3D::Object* object)
{
    return object->IsInstanceOf<Urho3D::Scene>() || object->IsInstanceOf<Urho3D::Node>();
}

QMimeData* SceneHierarchyWidget::ConstructMimeData(const QModelIndexList& indexes)
{
    QScopedPointer<SceneHierarchyMimeData> mime(new SceneHierarchyMimeData);
    QString text;
    for (const QModelIndex& index : indexes)
        if (Urho3D::Object* object = treeModel_->GetObject(index))
        {
            if (Urho3D::Node* node = dynamic_cast<Urho3D::Node*>(object))
            {
                mime->nodes_.push_back(node);
                text += QString::number(node->GetID());
            }
            else if (Urho3D::Component* component = dynamic_cast<Urho3D::Component*>(object))
            {
                mime->components_.push_back(component);
                text += QString::number(component->GetID());
            }
        }

    mime->setText(text);
    return mime.take();
}

bool SceneHierarchyWidget::CanDropMime(const QMimeData* data, const QModelIndex& parent, int row)
{
    using namespace Urho3D;
    const SceneHierarchyMimeData* mime = qobject_cast<const SceneHierarchyMimeData*>(data);
    if (!mime)
        return false;

    if (Node* parentNode = dynamic_cast<Node*>(treeModel_->GetObject(parent)))
    {
        // No node cycles
        for (Node* node : mime->nodes_)
            if (parentNode->IsChildOf(node))
                return false;

        // No component re-parenting
        if (mime->nodes_.empty())
        {
            for (Component* component : mime->components_)
                if (component->GetNode() != parentNode)
                    return false;
        }
    }

    return true;
}

bool SceneHierarchyWidget::DropMime(const QMimeData* data, const QModelIndex& parent, int row)
{
    using namespace Urho3D;
    const SceneHierarchyMimeData* mime = qobject_cast<const SceneHierarchyMimeData*>(data);
    if (!mime)
        return false;

    if (Node* parentNode = dynamic_cast<Node*>(treeModel_->GetObject(parent)))
    {
        QScopedPointer<QUndoCommand> group(new QUndoCommand);
        // \todo Use suppressUpdates_ to optimize operations
        if (!mime->nodes_.empty())
        {
            const unsigned numComponents = parentNode->GetNumComponents();
            if (row < (int)numComponents)
                row = (int)numComponents;

            for (Node* node : mime->nodes_)
            {
                Node* oldParent = node->GetParent();
                assert(oldParent);
                const unsigned oldIndex = oldParent->GetChildren().IndexOf(SharedPtr<Node>(node));
                const unsigned newIndex = (unsigned)row - numComponents;

                new NodeHierarchyAction(document_,
                    node->GetID(), oldParent->GetID(), oldIndex, parentNode->GetID(), newIndex,
                    group.data());

                ++row;
            }
        }
        else if (!mime->components_.empty())
        {
            // Re-order components
            for (Component* component : mime->components_)
            {
                Node* node = component->GetNode();
                const unsigned oldIndex = node->GetComponents().IndexOf(SharedPtr<Component>(component));

                new ComponentHierarchyAction(document_,
                    node->GetID(), component->GetID(), oldIndex, (unsigned)row,
                    group.data());

                ++row;
            }
        }

        document_.AddAction(group.take());
    }

    return true;
}

}
