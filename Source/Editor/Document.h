#pragma once

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

    /// Set title of the page.
    virtual void SetTitle(const QString& title);
    /// Launch file dialog and get the file name.
    virtual bool LaunchFileDialog(bool open);
    /// Open page from file.
    virtual bool Open();

    /// Return whether the page is active.
    bool IsActive() const;
    /// Return raw title of the page.
    QString GetRawTitle() { return title_; }
    /// Return file name of the page.
    QString GetFileName() { return fileName_; }
    /// Get main window.
    MainWindow& GetMainWindow() { return mainWindow_; }
    /// Get configuration.
    Configuration& GetConfig();

    /// Return title of the page.
    virtual QString GetTitle() { return title_; }
    /// Return whether the page can be saved.
    virtual bool CanBeSaved() { return false; }
    /// Return whether the page widget should be visible when the page is active.
    virtual bool IsPageWidgetVisible() { return true; }
    /// Return whether the Urho3D widget should be visible when the page is active.
    virtual bool IsUrho3DWidgetVisible() { return false; }
    /// Get name filters for open and save dialogs.
    virtual QString GetNameFilters() { return "All files (*.*)"; }

protected:
    /// Load the page from file.
    virtual bool DoLoad(const QString& fileName);

protected slots:
    /// Handle current page changed.
    virtual void HandleCurrentPageChanged(Document* page);

signals:
    /// Signals that title of the page has been changed.
    void titleChanged(Document* page);

private:
    /// Main window.
    MainWindow& mainWindow_;
    /// File name.
    QString fileName_;
    /// Title.
    QString title_;
};

}

