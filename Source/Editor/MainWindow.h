#pragma once

#include "Module.h"
#include "Urho3DProject.h"
#include "Widgets/Urho3DWidget.h"
#include <Urho3D/Core/Context.h>
#include <QApplication>
#include <QMainWindow>
#include <QMdiSubWindow>

class QDockWidget;
class QMainWindow;
class QDomNode;
class QVBoxLayout;
class QMdiArea;

namespace Urho3DEditor
{

class Configuration;
class Document;

/// Document window.
class DocumentWindow : public QMdiSubWindow
{
    Q_OBJECT

public:
    /// Construct.
    DocumentWindow(Document* document, QWidget* parent = nullptr);
    /// Get document.
    Document* GetDocument() const { return document_.data(); }

signals:
    /// Signals that document is about to be closed.
    void aboutToClose();

private slots:
    /// Update title.
    void updateTitle();

private:
    /// @see QWidget::closeEvent
    virtual void closeEvent(QCloseEvent *event) override;

private:
    /// Document.
    QScopedPointer<Document> document_;

};

/// Main window.
class MainWindow : public QObject
{
    Q_OBJECT

public:
    /// Contains name of layout file. Restart is required.
    static const QString VarLayoutFileName;

public:
    /// Construct.
    MainWindow(Configuration& config, QMainWindow& mainWindow);
    /// Destruct.
    virtual ~MainWindow();
    /// Initialize.
    virtual bool Initialize();
    /// Load layout.
    virtual void LoadLayout();

    /// Create Urho3D client widget.
    Urho3DClientWidget* CreateUrho3DClientWidget(QWidget* parent = nullptr);

    /// Get configuration.
    Configuration& GetConfig() const;
    /// Get current active document.
    Document* GetCurrentDocument() const;
    /// Get Urho3D widget.
    Urho3DWidget* GetUrho3DWidget() const;
    /// Get menu bar.
    QMenuBar* GetMenuBar() const;
    /// Get action by name.
    QAction* GetAction(const QString& name) const;
    /// Get menu.
    QMenu* GetMenu(const QString& name) const;

    /// Add menu action.
    QAction* AddAction(const QString& name, QAction* action);
    /// Add new menu action.
    QAction* AddAction(const QString& name, const QKeySequence& shortcut = QKeySequence());
    /// Add dock widget.
    void AddDock(Qt::DockWidgetArea area, QDockWidget* dock);

    /// Add document.
    void AddDocument(Document* document, bool bringToTop = true);

signals:
    /// Signals that current document has been changed.
    void currentDocumentChanged(Document* document);
    /// Signals that document is closed.
    void documentClosed(Document* document);
    /// Signals that menu is about to show.
    void updateMenu(QMenu* menu);

private:
    /// Initialize menu.
    void InitializeMenu();
    /// Read menu.
    QMenu* ReadMenu(const QDomNode& node);
    /// Read action.
    QAction* ReadAction(const QDomNode& node);

private slots:
    /// Close document.
    void CloseDocument(DocumentWindow* widget);
    /// Change document.
    void ChangeDocument(DocumentWindow* widget);

    /// Handle 'File/Exit'
    void HandleFileExit();
    /// Handle 'Edit/Undo'
    void EditUndo();
    /// Handle 'Edit/Redo'
    void EditRedo();
    /// Handle 'Tools/Options'
    void HandleToolsOptions();
    /// Handle 'Help/About'
    void HandleHelpAbout();
    /// Handle that menu is about to show.
    void HandleMenuAboutToShow();

private:
    /// Configuration.
    Configuration& config_;
    /// Main window.
    QMainWindow& mainWindow_;

    /// Central widget.
    QMdiArea* mdiArea_;
    /// Urho3D Widget.
    Urho3DHost* urhoHost_;

    /// Menu actions.
    QHash<QString, QAction*> menuActions_;
    /// Secondary and context menus.
    QHash<QString, QMenu*> menus_;

};

}

