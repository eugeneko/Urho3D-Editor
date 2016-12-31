#pragma once

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

// class ProjectManagerEvent : public QEvent
// {
// public:
//     enum Type
//     {
//         SetCurrentProject,
//     };
// };
// 
// /// Project manager.
// class ProjectManager
// {
//     Q_OBJECT
// 
// public:
//     /// Construct.
//     ProjectManager();
// };

/// Main window of Editor application.
class MainWindow : public QMainWindow, public Urho3D::Object
{
    Q_OBJECT
    URHO3D_OBJECT(MainWindow, Urho3D::Object);

public:
    /// Construct.
    MainWindow(Urho3D::Context* context);
    /// Destruct.
    ~MainWindow();

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

