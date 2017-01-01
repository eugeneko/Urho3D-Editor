#pragma once

#include "Module.h"
#include "Urho3DWidget.h"
#include "Urho3DProject.h"
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

    /// Get menu bar.
    QMenuBar* GetMenuBar() const;
    /// Get top-level menu.
    QMenu* GetTopLevelMenu(TopLevelMenu menu) const;
    /// Get menu action.
    QAction* GetMenuAction(MenuAction action) const;

    /// Add page.
    void AddPage(MainWindowPage* page, bool bringToTop = true);
    /// Select page.
    void SelectPage(MainWindowPage* page);
    /// Close page.
    void ClosePage(MainWindowPage* page);

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
    /// Handle tab title changed.
    virtual void HandleTabTitleChanged(MainWindowPage* page);

private:
    /// Main window.
    QMainWindow* mainWindow_;
    /// Urho3D context.
    Urho3D::Context* context_;

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
    MainWindowPage(Configuration& config);

    /// Handle the page selected.
    virtual void OnSelected() { }
    /// Set title of the page.
    virtual void SetTitle(const QString& title);
    /// Launch file dialog and get the file name.
    virtual bool LaunchFileDialog(bool open);
    /// Open page from file.
    virtual bool Open();

    /// Return raw title of the page.
    QString GetRawTitle() { return title_; }
    /// Return file name of the page.
    QString GetFileName() { return fileName_; }
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

signals:
    /// Signal for changing of page title.
    void titleChanged(MainWindowPage* page);

private:
    /// Configuration.
    Configuration& config_;
    /// File name.
    QString fileName_;
    /// Title.
    QString title_;
};

/// Main window of Editor application.
class MainWindow1 : public QMainWindow, public Urho3D::Object
{
    Q_OBJECT
    URHO3D_OBJECT(MainWindow1, Urho3D::Object);

public:
    /// Construct.
    MainWindow1(Urho3D::Context* context);
    /// Destruct.
    ~MainWindow1();

    /// Get client widget.
    Urho3DWidget* GetUrho3DWidget() { return urho3DWidget_; }
    /// Get current project.
    Urho3DProject* GetCurrentProject() { return urho3DProject_; }
    /// Set current project.
    void SetCurrentProject(Urho3D::SharedPtr<Urho3DProject> project);

    /// Get current page.
    AbstractPage* GetCurrentPage() const;

    /// Add new page.
    void AddPage(AbstractPage* page, bool makeActive = true);
    /// Close page.
    void ClosePage(AbstractPage* page);
    /// Close all pages.
    void CloseAllPages();

// public:
//     virtual void AddDocument(EditorDocument* document) override;
//     virtual QMenu* GetMainMenu(const String& name, const String& beforeName) override;

private:
    /// Update menu.
    void UpdateMenu();
    /// Find main menu by name.
    QMenu* FindMainMenu(const QString& name);

private slots:
    void OnTabRenamed(AbstractPage* page, const QString& title);
    void OnPageChanged(int index);

    void OnFileNewProject();
    void OnFileSave();
    void OnFileOpenProject();
    void OnFileCloseProject();

    void OnFileNewScene();
    void OnFileOpenScene();
    void OnFileSaveScene();
    void OnFileExit();

private:
    /// Central widget.
    QWidget* centralWidget_;
    /// Main window layout.
    QVBoxLayout* layout_;
    /// Tab bar widget.
    QTabBar* tabBar_;
    /// Urho3D Widget.
    Urho3DWidget* urho3DWidget_;
    /// Current project.
    Urho3D::SharedPtr<Urho3DProject> urho3DProject_;
    /// Pages.
    Urho3D::Vector<AbstractPage*> pages_;

    /// 'New Project' action.
    QAction* actionNewProject_;
    /// 'Save' action.
    QAction* actionSave_;
    /// 'Open Project' action.
    QAction* actionOpenProject_;
    /// 'Close Project' action.
    QAction* actionCloseProject_;
    /// 'Project Properties' action.
    QAction* actionProjectProperties_;

};

}

