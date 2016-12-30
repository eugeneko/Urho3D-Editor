#pragma once

#include "AbstractDocument.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>

namespace Urho3DEditor
{

class MainWindow;

class EditorPlugin : public QObject, public Urho3D::RefCounted
{
    Q_OBJECT

public:
    /// Destruct.
    virtual ~EditorPlugin() { }
    /// Register.
//     virtual void Register(EditorInterface& editor) = 0;
//     /// Unregister.
//     virtual void Unregister(EditorInterface& editor) = 0;
};

/// Main class of Editor application.
class Application : public QApplication
{
    Q_OBJECT

public:
    /// Construct.
    Application(int argc, char** argv, Urho3D::Context* context);
    /// Destruct.
    ~Application();
    /// Add plug-in.
    void AddPlugin(Urho3D::SharedPtr<EditorPlugin> plugin);
    /// Run!
    int Run();

private:
    /// Find main menu by name.
    QMenu* FindMainMenu(const QString& name);

private:
    /// Context.
    Urho3D::SharedPtr<Urho3D::Context> context_;
    /// Active directory.
    QString activeDirectory_;
    /// Main window.
    QScopedPointer<MainWindow> mainWindow_;

    /// Plug-ins.
    Urho3D::Vector<Urho3D::SharedPtr<EditorPlugin>> plugins_;
};

class SceneEditorPlugin : public EditorPlugin
{
    Q_OBJECT

public:
    /// Register.
//     virtual void Register(EditorInterface& editor) override;
//     /// Unregister.
//     virtual void Unregister(EditorInterface& editor) override;

private slots:
    /// Create new scene.
    void HandleNewScene();
};

}

