#include "Module.h"
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

void ModuleSystem::AddModule(const QString& name, Module* module)
{
    Q_ASSERT(module);
    modules_.insert(name, QSharedPointer<Module>(module));
    module->Initialize(*this);
}

void ModuleSystem::RemoveModule(const QString& name)
{
    modules_.remove(name);
}

Module* ModuleSystem::GetModule(const QString& name) const
{
    return modules_.value(name).data();
}

void Module::Initialize(ModuleSystem& system)
{
    system_ = &system;
    DoInitialize();
}


Module* Module::GetModule(const QString& name) const
{
    Q_ASSERT(system_);
    return system_->GetModule(name);
}

}
