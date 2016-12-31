#pragma once

#include <QHash>
#include <QObject>
#include <QSharedPointer>

namespace Urho3DEditor
{

class Module;

class ModuleSystem
{
public:
    /// Add module.
    void AddModule(Module* module);
    /// Remove module by name.
    void RemoveModule(const QString& name);
    /// Remove module.
    void RemoveModule(Module* module);
    /// Get module by name.
    Module* GetModule(const QString& name) const;
    /// Get module by type.
    template <class T>
    T* GetModule() const { return dynamic_cast<T*>(GetModule(T::staticMetaObject()->className())); }

private:
    /// Modules.
    QHash<QString, QSharedPointer<Module>> modules_;

};

/// Module of Urho3D Editor.
class Module : public QObject
{
    Q_OBJECT

public:
    /// Construct.
    Module(ModuleSystem& system);
    /// Get module by name.
    Module* GetModule(const QString& name) const { return system_.GetModule(name); }
    /// Get module by type.
    template <class T> T* GetModule() const { return system_.GetModule<T>(); }

private:
    /// Module system.
    ModuleSystem& system_;
};

}

