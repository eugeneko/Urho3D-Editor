#include "AttributeInspector.h"
#include "SceneDocument.h"
#include "SceneActions.h"
#include "../Core/Core.h"
#include "../Widgets/CollapsiblePanelWidget.h"
#include "../Widgets/AttributeWidgetImpl.h"
#include "../Widgets/SerializableWidget.h"
#include <Urho3D/Scene/Node.h>
#include <QDockWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>

namespace Urho3DEditor
{

AttributeInspector::AttributeInspector()
    : document_(nullptr)
    , suppressUpdates_(false)
    , generation_(0)
{
}

bool AttributeInspector::Initialize()
{
    Core& core = GetCore();

    // Connect to signals
    connect(&core, SIGNAL(currentDocumentChanged(Document*)), this, SLOT(HandleCurrentDocumentChanged(Document*)));
    connect(&core, SIGNAL(updateMenu(QMenu*)), this, SLOT(UpdateMenu()));

    // Create widget
    widget_.reset(new QDockWidget("Attribute Inspector"));
    widget_->hide();
    core.AddDock(Qt::RightDockWidgetArea, widget_.data());

    // Create actions
    showAction_.reset(core.AddAction("View.AttributeInspector"));
    showAction_->setCheckable(true);
    connect(showAction_.data(), SIGNAL(triggered(bool)), this, SLOT(ToggleShow(bool)));

    // Launch
    CreateBody();
    showAction_->activate(QAction::Trigger);
    return true;
}

void AttributeInspector::ToggleShow(bool checked)
{
    widget_->setVisible(checked);
}

void AttributeInspector::UpdateMenu()
{
    showAction_->setChecked(widget_->isVisible());
}

void AttributeInspector::HandleCurrentDocumentChanged(Document* document)
{
    if (document_)
    {
        disconnect(document_, 0, this, 0);
        document_ = nullptr;
        delete widget_->widget();
        serializableEditors_.clear();
    }
    if (SceneDocument* newDocument = dynamic_cast<SceneDocument*>(document))
    {
        document_ = newDocument;
        connect(document_, &SceneDocument::selectionChanged, this, &AttributeInspector::HandleSelectionChanged);
        connect(document_, &SceneDocument::attributeChanged, this, &AttributeInspector::HandleAttributeChanged);
        connect(document_, &SceneDocument::nodeTransformChanged, this, &AttributeInspector::HandleAttributeChanged);
        CreateBody();
    }
}

void AttributeInspector::HandleSelectionChanged()
{
    CreateBody();
}

void AttributeInspector::HandleAttributeChanged()
{
    if (suppressUpdates_)
        return;
    for (SerializableWidget* editor : serializableEditors_)
        editor->Update();
}

void AttributeInspector::HandleAttributeEdited(const SerializableVector& serializables,
    unsigned attributeIndex, const QVector<Urho3D::Variant>& newValues)
{
    assert(serializables.size() == newValues.size());

    using namespace Urho3D;
    if (!document_)
        return;
    suppressUpdates_ = true;

    const SerializableType type = GetSerializableType(*serializables[0]);
    QVector<EditSerializableAttributeAction> actions;
    actions.resize(serializables.size());

    // Gather IDs
    switch (type)
    {
    case SerializableType::Node:
        for (int i = 0; i < serializables.size(); ++i)
        {
            Node* node = dynamic_cast<Node*>(serializables[i]);
            assert(node);
            actions[i].serializableId_ = node->GetID();
        }
        break;
    case SerializableType::Component:
        for (int i = 0; i < serializables.size(); ++i)
        {
            Component* component = dynamic_cast<Component*>(serializables[i]);
            assert(component);
            actions[i].serializableId_ = component->GetID();
        }
        break;
    }

    // Gather values
    for (int i = 0; i < serializables.size(); ++i)
    {
        actions[i].oldValue_ = serializables[i]->GetAttribute(attributeIndex);
        actions[i].newValue_ = newValues[i];
    }

    // Add action
    document_->AddAction(new EditMultipleSerializableAttributeAction(*document_,
        type, generation_, attributeIndex, actions));
    
    suppressUpdates_ = false;
}

void AttributeInspector::HandleAttributeEditCommitted()
{
    ++generation_;
}

void AttributeInspector::CreateBody()
{
    using namespace Urho3D;
    serializableEditors_.clear();
    if (!widget_ || !document_)
        return;

    const SceneDocument::NodeSet& selectedNodes = document_->GetSelectedNodesAndComponents();

    QScopedPointer<QWidget> bodyWidget(new QWidget);
    QScopedPointer<QVBoxLayout> bodyLayout(new QVBoxLayout);

    if (!selectedNodes.empty())
    {
        QScopedPointer<CollapsiblePanelWidget> nodePanel(new CollapsiblePanelWidget(CreateNodePanelTitle(), true));
        QScopedPointer<SerializableWidget> nodePanelBody(CreateNodePanel());

        connect(&*nodePanelBody, &SerializableWidget::attributeChanged, this, &AttributeInspector::HandleAttributeEdited);
        connect(&*nodePanelBody, &SerializableWidget::attributeCommitted, this, &AttributeInspector::HandleAttributeEditCommitted);

        serializableEditors_.push_back(&*nodePanelBody);
        nodePanel->SetCentralWidget(nodePanelBody.take());
        bodyLayout->addWidget(nodePanel.take());
    }
    bodyLayout->addStretch(1);

    bodyWidget->setLayout(bodyLayout.take());
    widget_->setWidget(bodyWidget.take());
}

SerializableWidget* AttributeInspector::CreateNodePanel()
{
    using namespace Urho3D;
    const SceneDocument::NodeSet& selectedNodes = document_->GetSelectedNodesAndComponents();

    return new SerializableWidget(GatherSerializables(selectedNodes));
}

QString AttributeInspector::CreateNodePanelTitle()
{
    using namespace Urho3D;
    const SceneDocument::NodeSet& selectedNodes = document_->GetSelectedNodesAndComponents();

    QString nodesLabel;
    if (selectedNodes.size() == 1)
    {
        const Node* node = *selectedNodes.begin();
        nodesLabel = "Node (" + QString::number(node->GetID()) + ")";
    }
    else
    {
        // Gather node IDs
        QVector<unsigned> ids;
        for (const Node* node : selectedNodes)
            ids.push_back(node->GetID());
        qSort(ids);

        // Take first IDs
        static const int MAX_IDS = 1;
        nodesLabel = "Nodes (";
        for (int i = 0; i < qMin(ids.size(), MAX_IDS); ++i)
        {
            if (i != 0)
                nodesLabel += ", ";
            nodesLabel += QString::number(ids[i]);
        }

        // Put ellipsis if needed
        if (ids.size() > MAX_IDS)
            nodesLabel += ", ...";
        nodesLabel += ")";
    }

    return nodesLabel;
}

}
