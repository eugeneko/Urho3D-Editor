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

    /// Set title of the document.
    virtual void SetTitle(const QString& title);
    /// Launch file dialog and get the file name.
    virtual bool LaunchFileDialog(bool open);
    /// Open document from file.
    virtual bool Open();

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
    /// Nullptr will be returned only if document is not convertible to U.
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
    virtual QString GetTitle() { return title_; }
    /// Return whether the document can be saved.
    virtual bool CanBeSaved() { return false; }
    /// Return whether the document widget should be visible when the document is active.
    virtual bool IsPageWidgetVisible() { return true; }
    /// Return whether the Urho3D widget should be visible when the document is active.
    virtual bool IsUrho3DWidgetVisible() { return false; }
    /// Get name filters for open and save dialogs.
    virtual QString GetNameFilters() { return "All files (*.*)"; }

protected:
    /// Load the document from file.
    virtual bool DoLoad(const QString& fileName);

protected slots:
    /// Handle current document changed.
    virtual void HandleCurrentPageChanged(Document* document);

signals:
    /// Signals that title of the document has been changed.
    void titleChanged(Document* document);

private:
    /// Main window.
    MainWindow& mainWindow_;
    /// File name.
    QString fileName_;
    /// Title.
    QString title_;
    /// Stored objects.
    QHash<QString, QPointer<QObject>> objects_;

};

}

