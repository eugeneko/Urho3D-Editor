#pragma once

#include <QPointer>
#include <QWidget>
#include <type_traits>

namespace Urho3DEditor
{

class Configuration;
class Core;
class Document;

/// Document factory interface.
class DocumentFactory
{
public:
    /// Destructor.
    virtual ~DocumentFactory() {}
    /// Create document.
    virtual Document* CreateDocument(Core& core) const = 0;

    /// Returns document type name.
    virtual QString GetDocumentType() const = 0;
    /// Returns whether the document is save-able.
    virtual bool IsSaveable() const { return false; }
    /// Returns whether the document shall be saved when created.
    virtual bool ShallSaveOnCreate() const { return false; }
    /// Returns default save file name.
    virtual QString GetDefaultFileName() const { return ""; }
    /// Returns file name filters.
    virtual QStringList GetFileNameFilters() const = 0;

};

/// Document factory template.
template <class TDocument>
class DocumentFactoryT : public DocumentFactory
{
    static_assert(std::is_base_of<Document, TDocument>::value, "TDocument shall be derived from Document");

public:
    /// @see DocumentFactory::CreateDocument
    virtual Document* CreateDocument(Core& core) const override { return new TDocument(core); }
    /// @see DocumentFactory::GetDocumentType
    virtual QString GetDocumentType() const override { return TDocument::staticMetaObject.className(); }
};

/// Interface of document that can be placed as sub-window of main window area.
class Document : public QWidget
{
    Q_OBJECT

public:
    /// Construct.
    Document(Core& core);
    /// Destruct.
    virtual ~Document();

    /// Mark document as dirty.
    void MarkDirty();
    /// Reset document dirtiness.
    void ResetDirty();
    /// Returns whether the document is dirty.
    bool IsDirty() const { return dirty_; }

    /// Set title of the document.
    void SetTitle(const QString& title);
    /// Open document from file.
    virtual bool Open(const QString& fileName);
    /// Save document to file. Old file name is used if possible.
    virtual bool Save(const QString& fileName);

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
    /// Get editor core.
    Core& GetCore() { return core_; }
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
    /// Editor core.
    Core& core_;
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

