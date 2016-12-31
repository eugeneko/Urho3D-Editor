#pragma once

#include <QHash>
#include <QObject>
#include <QSharedPointer>

namespace Urho3DEditor
{

class Module;

/// Module system of Editor.
class ModuleSystem
{
public:
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
    /// Modules.
    QHash<QString, QSharedPointer<Module>> modules_;

};

/// Module of Urho3D Editor.
class Module : public QObject
{
    Q_OBJECT

public:
    /// Initialize module.
    void Initialize(ModuleSystem& system);

    /// Get module by name.
    Module* GetModule(const QString& name) const;
    /// Get module by type.
    template <class T> T* GetModule() const { return dynamic_cast<T*>(GetModule(T::staticMetaObject.className())); }

protected:
    /// Initialize module.
    virtual void DoInitialize() { }

private:
    /// Module system.
    ModuleSystem* system_;
};

}

