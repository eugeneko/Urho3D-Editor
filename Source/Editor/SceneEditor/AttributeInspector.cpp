#include "AttributeInspector.h"
#include "SceneDocument.h"
#include "../MainWindow.h"
#include <Urho3D/Scene/Node.h>
#include <QDockWidget>
#include <QGridLayout>
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

    connect(&mainWindow, SIGNAL(pageChanged(Document*)), this, SLOT(HandleCurrentPageChanged(Document*)));

    showAction_.reset(mainWindow.AddAction("View.AttributeInspector"));
    showAction_->setCheckable(true);
    connect(showAction_.data(), SIGNAL(triggered(bool)), this, SLOT(ToggleShow(bool)));

    showAction_->activate(QAction::Trigger);
    return true;
}

void AttributeInspector::ToggleShow(bool checked)
{
    if (checked)
    {
        MainWindow& mainWindow = GetMainWindow();
        widget_.reset(new QDockWidget("Attribute Inspector"));
        mainWindow.AddDock(Qt::RightDockWidgetArea, widget_.data());
        UpdateInspector();
    }
    else
    {
        widget_->close();
        widget_.reset();
    }
}

void AttributeInspector::UpdateMenu()
{

}

void AttributeInspector::HandleCurrentPageChanged(Document* document)
{
    if (document_)
    {
        disconnect(document_, 0, this, 0);
        document_ = nullptr;
    }
    if (SceneDocument* newDocument = dynamic_cast<SceneDocument*>(document))
    {
        document_ = newDocument;
        connect(document_, SIGNAL(selectionChanged()), this, SLOT(HandleSelectionChanged()));
        UpdateInspector();
    }
}

void AttributeInspector::HandleSelectionChanged()
{
    UpdateInspector();
}

void AttributeInspector::UpdateInspector()
{
    using namespace Urho3D;
    if (!widget_ || !document_)
        return;

    // Get selection
    const SceneDocument::NodeSet& selectedNodes = document_->GetSelectedNodesAndComponents();

    QScopedPointer<QGridLayout> layout(new QGridLayout);

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

    QScopedPointer<QWidget> body(new QWidget);
    body->setLayout(layout.data());
    widget_->setWidget(body.take());
}

}
