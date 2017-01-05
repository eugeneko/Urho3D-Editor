#pragma once

#include "Module.h"
#include "Urho3DProject.h"
#include "Widgets/Urho3DWidget.h"
#include <Urho3D/Core/Context.h>
#include <QApplication>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>

namespace Urho3DEditor
{

class Configuration;
class Document;

/// Main window.
class MainWindow : public QObject
{
    Q_OBJECT

public:
    /// Top-level menus.
    enum TopLevelMenu
    {
        MenuFile,
        MenuView,
        MenuTools,
        MenuHelp
    };
    /// Menu actions.
    enum MenuAction
    {
        MenuFileNew_After,
        MenuFileOpen_After,
        MenuFileClose,
        MenuFileSave,
        MenuFileSaveAs,
        MenuFileExit_Before,
        MenuFileExit,
        MenuToolsOptions,
        MenuHelpAbout_Before,
        MenuHelpAbout
    };
    /// Construct.
    MainWindow(Configuration& config, QMainWindow& mainWindow, Urho3D::Context& context);
    /// Initialize.
    virtual bool Initialize();

    /// Get configuration.
    Configuration& GetConfig() const;
    /// Get context.
    Urho3D::Context& GetContext() const;
    /// Get current active page.
    Document* GetCurrentPage() const;
    /// Get Urho3D widget.
    Urho3DWidget* GetUrho3DWidget() const;
    /// Get menu bar.
    QMenuBar* GetMenuBar() const;
    /// Get top-level menu.
    QMenu* GetTopLevelMenu(TopLevelMenu menu) const;
    /// Get menu action.
    QAction* GetMenuAction(MenuAction action) const;

    /// Add dock widget.
    void AddDock(Qt::DockWidgetArea area, QDockWidget* dock);

    /// Add page.
    void AddPage(Document* page, bool bringToTop = true);
    /// Select page.
    void SelectPage(Document* page);
    /// Close page.
    void ClosePage(Document* page);

signals:
    /// Signals that current page has been changed.
    void pageChanged(Document* page);
    /// Signals that page is closed.
    void pageClosed(Document* page);

protected:
    /// Initialize layout.
    virtual void InitializeLayout();
    /// Initialize menu.
    virtual void InitializeMenu();

protected slots:
    /// Handle 'File/Close'
    virtual void HandleFileClose();
    /// Handle 'File/Exit'
    virtual void HandleFileExit();
    /// Handle 'Tools/Options'
    virtual void HandleToolsOptions();
    /// Handle 'Help/About'
    virtual void HandleHelpAbout();
    /// Handle tab changed.
    virtual void HandleTabChanged(int index);
    /// Handle tab moved.
    virtual void HandleTabMoved(int from, int to);
    /// Handle tab closed.
    virtual void HandleTabClosed(int index);
    /// Handle tab title changed.
    virtual void HandleTabTitleChanged(Document* page);

private:
    /// Configuration.
    Configuration& config_;
    /// Main window.
    QMainWindow& mainWindow_;
    /// Urho3D context.
    Urho3D::Context& context_;

    /// Central widget.
    QScopedPointer<QWidget> widget_;
    /// Main window layout.
    QScopedPointer<QVBoxLayout> layout_;
    /// Tab bar widget.
    QScopedPointer<QTabBar> tabBar_;
    /// Urho3D Widget.
    QScopedPointer<Urho3DWidget> urho3DWidget_;

    /// Top-level menus.
    QHash<TopLevelMenu, QMenu*> topLevelMenus_;
    /// Menu actions.
    QHash<MenuAction, QAction*> menuActions_;

    /// Pages.
    QVector<Document*> pages_;

};

}

