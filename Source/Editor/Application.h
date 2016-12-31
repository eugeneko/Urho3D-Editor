#pragma once

#include "Module.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>

namespace Urho3DEditor
{

/// Main class of Editor application.
class Application : public QApplication
{
    Q_OBJECT

public:
    /// Construct.
    Application(int argc, char** argv, Urho3D::Context* context);
    /// Destruct.
    ~Application();
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
    QScopedPointer<QMainWindow> mainWindow_;
    /// Modules.
    ModuleSystem moduleSystem_;

};

}

