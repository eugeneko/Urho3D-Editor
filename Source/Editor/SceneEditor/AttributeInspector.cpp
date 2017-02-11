#include "AttributeInspector.h"
#include "SceneDocument.h"
#include "../MainWindow.h"
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
{
}

bool AttributeInspector::Initialize()
{
    MainWindow& mainWindow = GetMainWindow();

    // Connect to signals
    connect(&mainWindow, SIGNAL(currentDocumentChanged(Document*)), this, SLOT(HandleCurrentDocumentChanged(Document*)));

    // Create widget
    widget_.reset(new QDockWidget("Attribute Inspector"));
    widget_->hide();
    mainWindow.AddDock(Qt::RightDockWidgetArea, widget_.data());

    // Create actions
    showAction_.reset(mainWindow.AddAction("View.AttributeInspector"));
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
    }
    if (SceneDocument* newDocument = dynamic_cast<SceneDocument*>(document))
    {
        document_ = newDocument;
        connect(document_, SIGNAL(selectionChanged()), this, SLOT(HandleSelectionChanged()));
        CreateBody();
    }
}

void AttributeInspector::HandleSelectionChanged()
{
    CreateBody();
}

void AttributeInspector::CreateBody()
{
    using namespace Urho3D;
    if (!widget_ || !document_)
        return;

    const SceneDocument::NodeSet& selectedNodes = document_->GetSelectedNodesAndComponents();

    QScopedPointer<QWidget> bodyWidget(new QWidget);
    QScopedPointer<QVBoxLayout> bodyLayout(new QVBoxLayout);

    if (!selectedNodes.empty())
    {
        QScopedPointer<CollapsiblePanelWidget> nodePanel(new CollapsiblePanelWidget(CreateNodePanelTitle(), true));
        nodePanel->SetCentralWidget(CreateNodePanel());
        bodyLayout->addWidget(nodePanel.take());
    }
    bodyLayout->addStretch(1);

    bodyWidget->setLayout(bodyLayout.take());
    widget_->setWidget(bodyWidget.take());
}

QWidget* AttributeInspector::CreateNodePanel()
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
