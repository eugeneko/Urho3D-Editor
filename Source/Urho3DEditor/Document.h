#pragma once

#include <QPointer>
#include <QWidget>

namespace Urho3DEditor
{

class Configuration;
class Document;
class MainWindow;

/// Document of the main window.
class Document : public QWidget
{
    Q_OBJECT

public:
    /// Construct.
    Document(MainWindow& mainWindow);
    /// Destruct.
    virtual ~Document();

    /// Mark document as dirty.
    void MarkDirty();
    /// Reset document dirtiness.
    void ResetDirty();
    /// Set title of the document.
    virtual void SetTitle(const QString& title);
    /// Launch file dialog and get the file name.
    virtual bool LaunchFileDialog(bool open);
    /// Open document from file.
    virtual bool Open();
    /// Save document to file. Old file name is never used.
    virtual bool SaveAs();
    /// Save document to file. Old file name is used if possible.
    virtual bool Save();

    /// Undo.
    virtual void Undo() {}
    /// Redo.
    virtual void Redo() {}

    /// Return whether the document is active.
    bool IsActive() const;
    /// Return raw title of the document.
    QString GetRawTitle() { return title_; }
    /// Return file name of the document.
    QString GetFileName() { return fileName_; }
    /// Get main window.
    MainWindow& GetMainWindow() { return mainWindow_; }
    /// Get configuration.
    Configuration& GetConfig();
    /// Get (and create if not exist) object of specified type. Type shall be constructible from this casted to U&.
    /// Null will be returned only if document is not convertible to U.
    template <class TObject, class TDocument = Document, class TParent = QObject>
    TObject* Get(TParent* parent = nullptr)
    {
        TDocument* document = qobject_cast<TDocument*>(this);
        if (!document)
            return nullptr;

        const char* className = TObject::staticMetaObject.className();
        TObject* object = qobject_cast<TObject*>(objects_.value(className).data());
        if (!object)
        {
            object = new TObject(*document);
            if (parent)
                object->setParent(parent);
            else
                object->setParent(this);
            objects_.insert(className, object);
        }
        return object;
    }
    /// Remove object.
    template <class T>
    void Remove()
    {
        const char* className = T::staticMetaObject.className();
        objects_.remove(className);
    }

    /// Return title of the document.
    virtual QString GetTitle() { return title_ + (dirty_ ? " *" : ""); }
    /// Return whether the document can be saved.
    virtual bool CanBeSaved() { return false; }
    /// Return default file name for save.
    virtual QString GetDefaultName() { return ""; }
    /// Return name filters for open and save dialogs.
    virtual QString GetNameFilters() { return "All files (*.*)"; }

private:
    /// Load the document from file.
    virtual bool DoLoad(const QString& fileName);
    /// Save the document to file.
    virtual bool DoSave(const QString& fileName);

protected slots:
    /// Handle current document changed.
    virtual void HandleCurrentDocumentChanged(Document* document);

signals:
    /// Signals that title of the document has been changed.
    void titleChanged();

private:
    /// Main window.
    MainWindow& mainWindow_;
    /// File name.
    QString fileName_;
    /// Title.
    QString title_;
    /// Stored objects.
    QHash<QString, QPointer<QObject>> objects_;
    /// Dirty flag.
    bool dirty_ = false;

};

}

