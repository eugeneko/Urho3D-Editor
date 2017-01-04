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
class MainWindowPage;

/// Main window.
class MainWindow : public Module
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
        MenuHelpAbout_Before,
        MenuHelpAbout
    };
    /// Construct.
    MainWindow(QMainWindow* mainWindow, Urho3D::Context* context);

    /// Get configuration.
    Configuration& GetConfig() const;
    /// Get context.
    Urho3D::Context* GetContext() const;
    /// Get current active page.
    MainWindowPage* GetCurrentPage() const;
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
    void AddPage(MainWindowPage* page, bool bringToTop = true);
    /// Select page.
    void SelectPage(MainWindowPage* page);
    /// Close page.
    void ClosePage(MainWindowPage* page);

signals:
    /// Signals that current page has been changed.
    void pageChanged(MainWindowPage* page);
    /// Signals that page is closed.
    void pageClosed(MainWindowPage* page);

protected:
    /// Initialize module.
    virtual bool DoInitialize() override;
    /// Initialize layout.
    virtual void InitializeLayout();
    /// Initialize menu.
    virtual void InitializeMenu();

protected slots:
    /// Handle 'File/Close'
    virtual void HandleFileClose();
    /// Handle 'File/Exit'
    virtual void HandleFileExit();
    /// Handle 'Help/About'
    virtual void HandleHelpAbout();
    /// Handle tab changed.
    virtual void HandleTabChanged(int index);
    /// Handle tab moved.
    virtual void HandleTabMoved(int from, int to);
    /// Handle tab closed.
    virtual void HandleTabClosed(int index);
    /// Handle tab title changed.
    virtual void HandleTabTitleChanged(MainWindowPage* page);

private:
    /// Main window.
    QMainWindow* mainWindow_;
    /// Urho3D context.
    Urho3D::Context* context_;
    /// Configuration.
    Configuration* config_;

    /// Central widget.
    QScopedPointer<QWidget> centralWidget_;
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
    QVector<MainWindowPage*> pages_;

};

/// Page of the main window.
class MainWindowPage : public QWidget
{
    Q_OBJECT

public:
    /// Construct.
    MainWindowPage(MainWindow& mainWindow);

    /// Set title of the page.
    virtual void SetTitle(const QString& title);
    /// Launch file dialog and get the file name.
    virtual bool LaunchFileDialog(bool open);
    /// Open page from file.
    virtual bool Open();

    /// Return whether the page is active.
    bool IsActive() const;
    /// Return raw title of the page.
    QString GetRawTitle() { return title_; }
    /// Return file name of the page.
    QString GetFileName() { return fileName_; }
    /// Get main window.
    MainWindow& GetMainWindow() { return mainWindow_; }

    /// Return title of the page.
    virtual QString GetTitle() { return title_; }
    /// Return whether the page can be saved.
    virtual bool CanBeSaved() { return false; }
    /// Return whether the page widget should be visible when the page is active.
    virtual bool IsPageWidgetVisible() { return true; }
    /// Return whether the Urho3D widget should be visible when the page is active.
    virtual bool IsUrho3DWidgetVisible() { return false; }
    /// Get name filters for open and save dialogs.
    virtual QString GetNameFilters() { return "All files (*.*)"; }

protected:
    /// Load the page from file.
    virtual bool DoLoad(const QString& fileName);

protected slots:
    /// Handle current page changed.
    virtual void HandleCurrentPageChanged(MainWindowPage* page);

signals:
    /// Signals that title of the page has been changed.
    void titleChanged(MainWindowPage* page);

private:
    /// Main window.
    MainWindow& mainWindow_;
    /// File name.
    QString fileName_;
    /// Title.
    QString title_;
};

}

