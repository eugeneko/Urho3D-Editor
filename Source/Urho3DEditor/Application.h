#pragma once

#include "Module.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>

namespace Urho3DEditor
{

class Configuration;
class MainWindow;

/// Main class of Editor application.
class Application : public QApplication
{
    Q_OBJECT

public:
    /// Construct.
    Application(int argc, char** argv);
    /// Destruct.
    virtual ~Application();
    /// Run!
    virtual int Run();

protected:
    /// Initialize application.
    virtual bool Initialize();

protected:
    /// Main window widget.
    QScopedPointer<QMainWindow> mainWindowWidget_;
    /// Configuration.
    QScopedPointer<Configuration> config_;
    /// Main window manager.
    QScopedPointer<MainWindow> mainWindow_;
    /// Modules.
    QScopedPointer<ModuleSystem> moduleSystem_;

};

}

