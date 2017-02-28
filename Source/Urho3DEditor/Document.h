#pragma once

#include <QPointer>
#include <QWidget>

namespace Urho3DEditor
{

struct DocumentDescription;
class Configuration;
class Core;

/// Macro to define Document helper functions.
#define URHO3DEDITOR_DOCUMENT \
public: \
    static const DocumentDescription& GetStaticDescription() { static const DocumentDescription desc = CreateDescription(); return desc; } \
    static const QString& GetStaticName() { return GetStaticDescription().typeName_; } \
    virtual const DocumentDescription& GetDescription() const override { return GetStaticDescription(); }

/// Interface of document that can be placed as sub-window of main window area.
class Document : public QWidget
{
    Q_OBJECT

public:
    /// Construct.
    Document(Core& core);
    /// Destruct.
    virtual ~Document();
    /// Get description.
    virtual const DocumentDescription& GetDescription() const = 0;

    /// Mark document as dirty.
    void MarkDirty();
    /// Reset document dirtiness.
    void ResetDirty();
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

/// Document description.
struct DocumentDescription
{
    /// Factory callback.
    using Factory = Document*(*)(Core& core);
    /// Construct default.
    DocumentDescription() {}
    /// Construct.
    DocumentDescription(const QString& typeName, Factory factory) : typeName_(typeName), factory_(factory) {}

    /// Document type.
    QString typeName_;
    /// Factory.
    Factory factory_ = nullptr;

    /// Whether the document is save-able.
    bool saveable_ = false;
    /// Whether the document shall be saved on creation before further actions.
    bool saveOnCreate_ = false;
    /// Default save file name.
    QString defaultFileName_;
    /// File name filters.
    QStringList fileNameFilters_;
};

/// Helper wrapper for DocumentDescription.
template <class TDocument>
struct DocumentDescriptionT : DocumentDescription
{
    /// Construct.
    DocumentDescriptionT()
        : DocumentDescription(TDocument::staticMetaObject.className(), [](Core& core) -> Document* { return new TDocument(core); })
    {
    }
};

}

