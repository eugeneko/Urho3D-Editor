#pragma once

#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <Urho3D/Core/Object.h>

namespace Urho3DEditor
{

class Configuration;
class Core;
class Module;

/// Module system of Editor.
class ModuleSystem
{
public:
    /// Construct.
    ModuleSystem(Configuration& config, Core& core);

    /// Get config.
    Configuration& GetConfig() { return config_; }
    /// Get editor core.
    Core& GetCore() { return core_; }

    /// Add module. Ownership is passed to ModuleSystem.
    void AddModule(const QString& name, Module* module);
    /// Remove module by name.
    void RemoveModule(const QString& name);
    /// Get module by name.
    Module* GetModule(const QString& name) const;

    /// Add module by type. Ownership is passed to ModuleSystem.
    template <class T>
    void AddModule(T* module) { AddModule(T::staticMetaObject.className(), module); }
    /// Remove module by type.
    template <class T>
    void RemoveModule() { RemoveModule(T::staticMetaObject.className()); }
    /// Get module by type.
    template <class T>
    T* GetModule() const { return dynamic_cast<T*>(GetModule(T::staticMetaObject.className())); }

private:
    /// Configuration.
    Configuration& config_;
    /// Editor core.
    Core& core_;
    /// Modules.
    QHash<QString, QSharedPointer<Module>> modules_;

};

/// Module of Urho3D Editor. Mustn't be used before Initialize() call.
class Module : public QObject
{
    Q_OBJECT

public:
    /// Attach module to system.
    void Construct(ModuleSystem& system);
    /// Initialize module.
    virtual bool Initialize() { return true; }

    /// Get config.
    Configuration& GetConfig() { return system_->GetConfig(); }
    /// Get editor core.
    Core& GetCore() { return system_->GetCore(); }
    /// Get module by name.
    Module* GetModule(const QString& name) const;
    /// Get module by type.
    template <class T> T* GetModule() const { return dynamic_cast<T*>(GetModule(T::staticMetaObject.className())); }

private:
    /// Module system.
    ModuleSystem* system_;
};

}

