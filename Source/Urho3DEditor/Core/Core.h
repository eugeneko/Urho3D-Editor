#pragma once

#include "TypeMap.h"
#include "../Module.h"
#include "../Urho3DProject.h"
#include "Widgets/Urho3DWidget.h"
#include <Urho3D/Core/Context.h>
#include <QApplication>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QMessageBox>

class QDockWidget;
class QMainWindow;
class QDomNode;
class QVBoxLayout;
class QMdiArea;

namespace Urho3DEditor
{

class Configuration;
class Document;
class DocumentWindow;

/// Core singleton of Urho3D Editor.
/// #TODO Add 'Unsaved changes' message on main window close.
class Core : public QObject
{
    Q_OBJECT

public:
    /// Contains name of layout file. Restart is required.
    static const QString VarLayoutFileName;

public:
    /// Construct.
    Core(Configuration& config, QMainWindow& mainWindow);
    /// Destruct.
    virtual ~Core();

    /// Show error message.
    QMessageBox::StandardButton Error(const QString& text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::Ok);

    /// Register document.
    bool RegisterDocument(const DocumentDescription& desc);
    /// Register filter for Open dialog.
    bool RegisterFilter(const QString& filter, const QStringList& documentTypes);
    /// Create new document by type name.
    bool NewDocument(const QString& documentType);
    /// Open document file. If type list is not empty, only specified document types may be loaded.
    bool OpenDocument(const QString& fileName, QStringList documentTypes = QStringList());
    /// Launch open dialog and open selected documents. If type name is not specified, all document types are allowed.
    bool OpenDocumentDialog(const QString& documentType, bool allowMultiselect);
    /// Save document. Launch save dialog if file name is unknown.
    bool SaveDocument(Document& document, bool saveAs = false);
    /// Close document. Returns true if close operation was successful.
    bool CloseDocument(DocumentWindow& documentWindow);
    /// Set current project.
    void SetCurrentProject(QSharedPointer<Urho3DProject> project);

    /// Create new document by type.
    template <class T> bool NewDocument() { return NewDocument(T::staticMetaObject.className()); }
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

private slots:
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
    /// Application parameters.
    Urho3D::VariantMap applicationParameters_;

    /// Registered documents.
    TypeMap<DocumentDescription> registeredDocuments_;
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
    QSharedPointer<Urho3DProject> currentProject_;

    /// Menu actions.
    QHash<QString, QAction*> menuActions_;
    /// Secondary and context menus.
    QHash<QString, QMenu*> menus_;

};

}

