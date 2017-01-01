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

ModuleSystem::ModuleSystem(Urho3D::Context* context)
    : context_(context)
{

}

void ModuleSystem::AddModule(const QString& name, Module* module)
{
    Q_ASSERT(module);
    if (module->Initialize(*this))
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

bool Module::Initialize(ModuleSystem& system)
{
    system_ = &system;
    return DoInitialize();
}


Module* Module::GetModule(const QString& name) const
{
    Q_ASSERT(system_);
    return system_->GetModule(name);
}

}
