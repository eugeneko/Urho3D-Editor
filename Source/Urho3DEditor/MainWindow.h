#pragma once

#include "Module.h"
#include "Urho3DProject.h"
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

/// Ordered map from type to object.
template <class T>
class TypeMap
{
public:
    /// Iterator.
    using Iterator = typename QVector<QPair<QString, T>>::ConstIterator;

    /// Insert new value.
    bool Insert(const QString& typeName, const T& value)
    {
        if (map_.contains(typeName))
            return false;
        map_.insert(typeName, storage_.size());
        storage_.push_back(qMakePair(typeName, value));
        return true;
    }
    /// Find value by type name.
    const T* Find(const QString& typeName) const
    {
        const int index = map_.value(typeName, -1);
        return index >= 0 && index < storage_.size() ? &storage_[index].second : nullptr;
    }
    /// Get value by type name.
    const T& Get(const QString& typeName)
    {
        const T* value = Find(typeName);
        assert(value);
        return value->second;
    }
    /// Begin iterator.
    Iterator Begin() const { return storage_.cbegin(); }
    /// End iterator.
    Iterator End() const { return storage_.cend(); }

private:
    /// Vector storage.
    QVector<QPair<QString, T>> storage_;
    /// Map.
    QHash<QString, int> map_;
};

template <class T> typename TypeMap<T>::Iterator begin(const TypeMap<T>& map) { return map.Begin(); }

template <class T> typename TypeMap<T>::Iterator end(const TypeMap<T>& map) { return map.End(); }

class Core;

/// Document window. Owned document mustn't be null and destroyed in destructor.
/// #TODO Move to separate file
class DocumentWindow : public QMdiSubWindow
{
    Q_OBJECT

public:
    /// Construct.
    DocumentWindow(Core& core, Document* document, QWidget* parent = nullptr);
    /// Get document.
    Document& GetDocument() const { return *document_; }

private slots:
    /// Update title.
    void updateTitle();

private:
    /// @see QWidget::closeEvent
    virtual void closeEvent(QCloseEvent *event) override;

private:
    /// Core.
    Core& core_;
    /// Document.
    QScopedPointer<Document> document_;

};

/// Core singleton of Urho3D Editor.
/// #TODO Rename files
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

    /// Menu actions.
    QHash<QString, QAction*> menuActions_;
    /// Secondary and context menus.
    QHash<QString, QMenu*> menus_;

};

}

