#pragma once

#include "Urho3DWidget.h"
#include <Urho3D/Core/Context.h>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QTabWidget>

namespace Urho3D
{

class EditorDocument;

/// Main window of Editor application.
class MainWindow : public QMainWindow, public Object
{
    Q_OBJECT
    URHO3D_OBJECT(MainWindow, Object);

public:
    /// Construct.
    MainWindow(Context* context);
    /// Destruct.
    ~MainWindow();
    /// Get client widget.
    Urho3DWidget* GetUrho3DWidget() { return urho3DWidget_.data(); }
    /// Add document.
    void AddDocument(EditorDocument* document, bool bringToTop);

// public:
//     virtual void AddDocument(EditorDocument* document) override;
//     virtual QMenu* GetMainMenu(const String& name, const String& beforeName) override;

private:
    /// Deactivate active document and activate new document.
    void ActivateDocument(EditorDocument* document);
    /// Find main menu by name.
    QMenu* FindMainMenu(const QString& name);

private slots:
    void OnTabChanged(int index);

    void OnFileNewScene(bool);
    void OnFileExit(bool);

private:
    /// Main Urho3D Widget.
    QScopedPointer<Urho3DWidget> urho3DWidget_;
    /// Main window tabs.
    QTabWidget* tabWidget_;

    /// Active document.
    EditorDocument* activeDocument_;
};

}

