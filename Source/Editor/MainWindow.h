#pragma once

#include "Module.h"
#include "Urho3DProject.h"
#include "Widgets/Urho3DWidget.h"
#include <Urho3D/Core/Context.h>
#include <QApplication>
#include <QMainWindow>

class QDockWidget;
class QMainWindow;
class QDomNode;
class QVBoxLayout;

namespace Urho3DEditor
{

class Configuration;
class Document;

/// Main window.
class MainWindow : public QObject
{
    Q_OBJECT

public:
    /// Contains name of layout file. Restart is required.
    static const QString VarLayoutFileName;

public:
    /// Construct.
    MainWindow(Configuration& config, QMainWindow& mainWindow, Urho3D::Context& context);
    /// Initialize.
    virtual bool Initialize();
    /// Load layout.
    virtual void LoadLayout();

    /// Get configuration.
    Configuration& GetConfig() const;
    /// Get context.
    Urho3D::Context& GetContext() const;
    /// Get current active document.
    Document* GetCurrentPage() const;
    /// Get Urho3D widget.
    Urho3DWidget* GetUrho3DWidget() const;
    /// Get menu bar.
    QMenuBar* GetMenuBar() const;
    /// Get action by name.
    QAction* GetAction(const QString& name) const;

    /// Add menu action.
    QAction* AddAction(const QString& name, QAction* action);
    /// Add new menu action.
    QAction* AddAction(const QString& name, const QKeySequence& shortcut = QKeySequence());
    /// Add dock widget.
    void AddDock(Qt::DockWidgetArea area, QDockWidget* dock);

    /// Add document.
    void AddPage(Document* document, bool bringToTop = true);
    /// Select document.
    void SelectPage(Document* document);
    /// Close document.
    void ClosePage(Document* document);

signals:
    /// Signals that current document has been changed.
    void pageChanged(Document* document);
    /// Signals that document is closed.
    void pageClosed(Document* document);

private:
    /// Initialize layout.
    void InitializeLayout();
    /// Initialize menu.
    void InitializeMenu();
    /// Read menu.
    QMenu* ReadMenu(const QDomNode& node);
    /// Read action.
    QAction* ReadAction(const QDomNode& node);

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
    virtual void HandleTabTitleChanged(Document* document);

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

    /// Menu actions.
    QHash<QString, QAction*> menuActions_;

    /// Pages.
    QVector<Document*> documents_;

};

}

