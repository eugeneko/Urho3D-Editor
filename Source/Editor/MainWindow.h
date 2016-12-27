#pragma once

#include <Urho3D/Core/Context.h>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>

namespace Urho3D
{

/// Main window of Editor application.
class MainWindow : public Object, public QMainWindow
{
    URHO3D_OBJECT(MainWindow, Object);

public:
    /// Construct.
    MainWindow(Context* context);
    /// Destruct.
    ~MainWindow();
    /// Create widgets.
    void CreateWidgets();
    /// Get client widget.
    QWidget* GetClientWidget() { return clientWidget_; }

// public:
//     virtual void AddDocument(EditorDocument* document) override;
//     virtual QMenu* GetMainMenu(const String& name, const String& beforeName) override;

private:
    /// Initialize engine.
    void InitializeEngine(QWidget* host);
    /// Find main menu by name.
    QMenu* FindMainMenu(const QString& name);

private:
    QWidget* clientWidget_;
};

}

