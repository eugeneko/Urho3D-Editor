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

Module::Module(ModuleSystem& system)
    : system_(system)
{
}

void ModuleSystem::AddModule(Module* module)
{
    Q_ASSERT(module);
    modules_.insert(module->metaObject()->className(), QSharedPointer<Module>(module));
}

void ModuleSystem::RemoveModule(const QString& name)
{
    modules_.remove(name);
}

void ModuleSystem::RemoveModule(Module* module)
{
    Q_ASSERT(module);
    RemoveModule(module->metaObject()->className());
}

Module* ModuleSystem::GetModule(const QString& name) const
{
    return modules_.value(name).data();
}

}
