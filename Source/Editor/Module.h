#pragma once

#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <Urho3D/Core/Object.h>

namespace Urho3DEditor
{

class Module;

/// Module system of Editor.
class ModuleSystem
{
public:
    /// Construct.
    ModuleSystem(Urho3D::Context* context);

    /// Get context.
    Urho3D::Context* GetContext() const { return context_; }

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
    /// Context.
    Urho3D::Context* context_;
    /// Modules.
    QHash<QString, QSharedPointer<Module>> modules_;

};

/// Module of Urho3D Editor.
class Module : public QObject
{
    Q_OBJECT

public:
    /// Initialize module.
    bool Initialize(ModuleSystem& system);

    /// Get context.
    Urho3D::Context* GetContext() { return system_->GetContext(); }
    /// Get module by name.
    Module* GetModule(const QString& name) const;
    /// Get module by type.
    template <class T> T* GetModule() const { return dynamic_cast<T*>(GetModule(T::staticMetaObject.className())); }

protected:
    /// Initialize module.
    virtual bool DoInitialize() { return true; }

private:
    /// Module system.
    ModuleSystem* system_;
};

}

