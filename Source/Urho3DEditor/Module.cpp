#include "Module.h"
#include <QMessageBox>
// #include "MainWindow.h"
// #include "EditorSettings.h"
// #include <Urho3D/Core/ProcessUtils.h>
// #include <Urho3D/Engine/EngineDefs.h>
// #include <QFile>
// #include <QHBoxLayout>
// #include <QTabBar>
// #include <QTimer>
//
// #include <Urho3D/Urho3DAll.h>

namespace Urho3DEditor
{

ModuleSystem::ModuleSystem(Configuration& config, MainWindow& mainWindow)
    : config_(config)
    , mainWindow_(mainWindow)
{

}

void ModuleSystem::AddModule(const QString& name, Module* module)
{
    Q_ASSERT(module);
    module->Construct(*this);
    if (module->Initialize())
        modules_.insert(name, QSharedPointer<Module>(module));
    else
    {
        delete module;

        QMessageBox messageBox;
        messageBox.setText("Failed to initialize module '" + name + "'");
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.exec();
    }
}

void ModuleSystem::RemoveModule(const QString& name)
{
    modules_.remove(name);
}

Module* ModuleSystem::GetModule(const QString& name) const
{
    return modules_.value(name).data();
}

void Module::Construct(ModuleSystem& system)
{
    system_ = &system;
}


Module* Module::GetModule(const QString& name) const
{
    Q_ASSERT(system_);
    return system_->GetModule(name);
}

}
