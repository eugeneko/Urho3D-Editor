#pragma once

#include "Module.h"
#include "Widgets/TreeModel.h"
#include <QDockWidget>

class QTreeView;
class QVBoxLayout;

namespace Urho3D
{

class Node;

}

namespace Urho3DEditor
{

class Configuration;
class MainWindow;
class MainWindowPage;
class HierarchyWindow;
class ScenePage;

/// Scene Editor module.
class HierarchyWindowManager : public Module
{
    Q_OBJECT

public:
    /// Construct.
    HierarchyWindowManager();

protected:
    /// Initialize module.
    virtual bool DoInitialize() override;

protected slots:
    /// Handle 'View/Hierarchy Window'
    virtual void HandleViewHierarchyWindow(bool checked);

private:
    /// Configuration.
//     Configuration* config_;
    /// Main window.
    MainWindow* mainWindow_;
    /// 'View/Hierarchy Window' action.
    QScopedPointer<QAction> actionViewHierarchyWindow_;

    /// Hierarchy Window.
    QScopedPointer<HierarchyWindow> hierarchyWindow_;

};

class HierarchyWindow : public QDockWidget
{
    Q_OBJECT

public:
    /// Construct.
    HierarchyWindow(MainWindow& mainWindow);

private:
    /// Rebuild hierarchy of specified scene and root node.
    virtual void RebuildHierarchy(ScenePage* page, Urho3D::Node& root);
    /// Rebuild item of hierarchy.
    virtual TreeItem* BuildTreeItems(Urho3D::Node& node, TreeItem* parent) const;

private slots:
    /// Handle current page changed.
    virtual void HandleCurrentPageChanged(MainWindowPage* page);
    /// Handle page closed.
    virtual void HandlePageClosed(MainWindowPage* page);

private:
    /// Main window.
    MainWindow& mainWindow_;
    /// Tree models.
    QHash<ScenePage*, QSharedPointer<TreeModel>> trees_;

    /// Layout.
    QScopedPointer<QVBoxLayout> layout_;
    /// Tree view.
    QScopedPointer<QTreeView> treeView_;

};

}

