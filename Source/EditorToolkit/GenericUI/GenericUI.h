#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/HashSet.h>

namespace Urho3D
{

class GenericUIHost;
class Scene;
class Node;
class Serializable;
class Selection;

class GenericWidget : public Object
{
    URHO3D_OBJECT(GenericWidget, Object);

public:
    GenericWidget(Context* context) : Object(context) { }
    void SetHost(GenericUIHost* host) { host_ = host; OnHostInitialized(); }
    GenericWidget* CreateChild(StringHash type);
    template <class T> T* CreateChild() { return dynamic_cast<T*>(CreateChild(T::GetTypeStatic())); }

protected:
    virtual void OnHostInitialized() { }
    virtual void OnChildAdded(GenericWidget* widget);

private:
    GenericUIHost* host_ = nullptr;
    Vector<SharedPtr<GenericWidget>> children_;

};

class GenericDialog : public GenericWidget
{
    URHO3D_OBJECT(GenericDialog, GenericWidget);

public:
    GenericDialog(Context* context) : GenericWidget(context) { }
    virtual void SetName(const String& name) = 0;
};

class GenericHierarchyListItem : public GenericWidget
{
    URHO3D_OBJECT(GenericHierarchyListItem, GenericWidget);

public:
    GenericHierarchyListItem(Context* context) : GenericWidget(context) { }
    virtual void SetText(const String& text) = 0;
};

class GenericHierarchyList : public GenericWidget
{
    URHO3D_OBJECT(GenericHierarchyList, GenericWidget);

public:
    GenericHierarchyList(Context* context) : GenericWidget(context) { }
};

class GenericUIHost : public Object
{
    URHO3D_OBJECT(GenericUIHost, Object);

public:
    GenericUIHost(Context* context) : Object(context) { }
    GenericWidget* CreateWidget(StringHash type);
    template <class T> T* CreateWidget() { return dynamic_cast<T*>(CreateWidget(T::GetTypeStatic())); }

protected:
    virtual GenericWidget* CreateWidgetImpl(StringHash type) = 0;
};

class GenericUI : public Object
{
    URHO3D_OBJECT(GenericUI, Object);

public:
    GenericUI(Context* context) : Object(context) { }
    void SetDefaultHost(GenericUIHost* host) { defaultHost_.Reset(host); }
    GenericUIHost* GetDefaultHost() { return defaultHost_.Get(); }

private:
    UniquePtr<GenericUIHost> defaultHost_;
};

}
