#include "AttributeInspector.h"
#include "SceneDocument.h"
#include "../MainWindow.h"
#include "../Widgets/CollapsiblePanelWidget.h"
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

    QScopedPointer<QWidget> bodyWidget(new QWidget);
    QScopedPointer<QVBoxLayout> bodyLayout(new QVBoxLayout);

    QScopedPointer<CollapsiblePanelWidget> nodePanel(new CollapsiblePanelWidget("Node", true));
    nodePanel->SetContentLayout(CreateNodePanel());
    bodyLayout->addWidget(nodePanel.take());
    bodyLayout->addStretch(1);

    bodyWidget->setLayout(bodyLayout.take());
    widget_->setWidget(bodyWidget.take());
}

QGridLayout* AttributeInspector::CreateNodePanel()
{
    using namespace Urho3D;
    const SceneDocument::NodeSet& selectedNodes = document_->GetSelectedNodesAndComponents();

    QScopedPointer<QGridLayout> layout(new QGridLayout());

    // List nodes
    QString nodesLabel;
    if (selectedNodes.empty())
    {
        nodesLabel = "Nothing selected";
    }
    else if (selectedNodes.size() == 1)
    {
        const Node* node = *selectedNodes.begin();
        nodesLabel = "Node (ID " + QString::number(node->GetID()) + ")";
    }
    else
    {
        // Gather node IDs
        QVector<unsigned> ids;
        for (const Node* node : selectedNodes)
            ids.push_back(node->GetID());
        qSort(ids);

        // Take first IDs
        static const int MAX_IDS = 5;
        nodesLabel = "Nodes (IDs ";
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
    layout->addWidget(new QLabel(nodesLabel), 0, 0);
    layout->setRowStretch(1, 1);

    return layout.take();
}

}
