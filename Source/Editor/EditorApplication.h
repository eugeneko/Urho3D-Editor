#pragma once

#include "EditorDocument.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>

namespace Urho3D
{

class MainWindow;

class EditorInterface
{
public:
    /// Add tab.
    virtual void AddDocument(EditorDocument* document) = 0;
    virtual QMenu* GetMainMenu(const String& name, const String& beforeName = "") = 0;
};

class EditorPlugin : public QObject, public RefCounted
{
    Q_OBJECT

public:
    /// Destruct.
    virtual ~EditorPlugin() { }
    /// Register.
    virtual void Register(EditorInterface& editor) = 0;
    /// Unregister.
    virtual void Unregister(EditorInterface& editor) = 0;
};

/// Main class of Editor application.
class EditorApplication : public QApplication, public EditorInterface
{
    Q_OBJECT

public:
    /// Construct.
    EditorApplication(int argc, char** argv, Context* context);
    /// Destruct.
    ~EditorApplication();
    /// Add plug-in.
    void AddPlugin(SharedPtr<EditorPlugin> plugin);
    /// Run!
    int Run();

public:
    virtual void AddDocument(EditorDocument* document) override;
    virtual QMenu* GetMainMenu(const String& name, const String& beforeName) override;

private:
    /// Find main menu by name.
    QMenu* FindMainMenu(const QString& name);

private:
    /// Context.
    SharedPtr<Context> context_;
    /// Active directory.
    QString activeDirectory_;
    /// Main window.
    QScopedPointer<MainWindow> mainWindow_;

    /// Plug-ins.
    Vector<SharedPtr<EditorPlugin>> plugins_;
};

class SceneEditorPlugin : public EditorPlugin
{
    Q_OBJECT

public:
    /// Register.
    virtual void Register(EditorInterface& editor) override;
    /// Unregister.
    virtual void Unregister(EditorInterface& editor) override;

private slots:
    /// Create new scene.
    void HandleNewScene();
};

}

