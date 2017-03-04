#pragma once

#include "TypeMap.h"
#include "Project.h"
#include "../Module.h"
#include "Widgets/Urho3DWidget.h"
#include <Urho3D/Core/Context.h>
#include <QApplication>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QSettings>

class QDockWidget;
class QMainWindow;
class QDomNode;
class QVBoxLayout;
class QMdiArea;

namespace Urho3DEditor
{

class GlobalVariable;
class Configuration;
class Document;
class DocumentWindow;

/// Core singleton of Urho3D Editor.
/// #TODO Add 'Unsaved changes' message on main window close.
class Core : public QObject
{
    Q_OBJECT

public:
    /// Construct.
    Core(Configuration& config, QMainWindow& mainWindow);
    /// Destruct.
    virtual ~Core();

    /// Show error message.
    QMessageBox::StandardButton Error(const QString& text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::Ok);
    /// Quit.
    void Quit();

    /// Create new project.
    bool NewProject();
    /// Open existing project.
    bool OpenProject(QString fileName = "");
    /// Close current project.
    void CloseProject();
    /// Get current project.
    Project* GetProject() { return currentProject_.data(); }

    /// Get document factory by type name.
    DocumentFactory* GetDocumentFactory(const QString& documentType) const;
    /// Get document factory by type.
    template <class T> DocumentFactory* GetDocumentFactory() const { return GetDocumentFactory(T::staticMetaObject.className()); }
    /// Get document factory by type of document.
    DocumentFactory* GetDocumentFactory(Document& document) const;
    /// Register document.
    bool RegisterDocument(DocumentFactory* factory);
    /// Register filter for Open dialog.
    bool RegisterFilter(const QString& filter, const QStringList& documentTypes);
    /// Register global variable.
    void RegisterGlobalVariable(GlobalVariable& variable);
    /// Get global variables.
    const QVector<GlobalVariable*>& GetGlobalVariables() const { return globalVariables_; }
    /// Save global variables.
    void SaveGlobalVariables();

    /// Create new document by type name.
    bool NewDocument(const QString& documentType);
    /// Create new document by type.
    template <class T> bool NewDocument() { return NewDocument(T::staticMetaObject.className()); }

    /// Open document file. If type list is not empty, only specified document types may be loaded.
    bool OpenDocument(const QString& fileName, QStringList documentTypes = QStringList());
    /// Launch open dialog and open selected documents. If type name is not specified, all document types are allowed.
    bool OpenDocumentDialog(const QString& documentType, bool allowMultiselect);
    /// Save document. Launch save dialog if file name is unknown.
    bool SaveDocument(Document& document, bool saveAs = false);
    /// Close document. Returns true if close operation was successful.
    bool CloseDocument(DocumentWindow& documentWindow);
    /// Close current document.
    bool CloseCurrentDocument();
    /// Close all documents.
    bool CloseAllDocuments();

    /// Launch generic open dialog and try to open all selected files.
    bool Open() { return OpenDocumentDialog("", true); }

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
    Urho3DWidget& GetUrho3DWidget() const;
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
    /// Update window title.
    void UpdateWindowTitle();
    /// Update project-specific context.
    void UpdateProjectContext();

private slots:
    /// Change document.
    void ChangeDocument(DocumentWindow* widget);

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
    /// Settings.
    QSettings settings_;
    /// Variables grouped by sections.
    QVector<GlobalVariable*> globalVariables_;

    /// Configuration.
    Configuration& config_;
    /// Main window.
    QMainWindow& mainWindow_;
    /// Application parameters.
    Urho3D::VariantMap applicationParameters_;

    /// Registered documents.
    TypeMap<QSharedPointer<DocumentFactory>> registeredDocuments_;
    /// Registered document filters.
    QStringList registeredDocumentFilters_;
    /// Registered document filters mapped to document type.
    QHash<QString, QStringList> filterToDocumentType_;

    /// Central widget.
    QMdiArea* mdiArea_;
    /// Urho3D Widget.
    Urho3DHost* urhoHost_;
    /// Current document. Used to suppress redundant notifications.
    DocumentWindow* currentDocument_ = nullptr;
    /// Current project.
    QScopedPointer<Project> currentProject_;

    /// Menu actions.
    QHash<QString, QAction*> menuActions_;
    /// Secondary and context menus.
    QHash<QString, QMenu*> menus_;

};

}

