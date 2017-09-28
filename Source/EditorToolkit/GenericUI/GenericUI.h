#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/HashSet.h>

namespace Urho3D
{

class GenericUIHost;
class Scene;
class Node;
class Serializable;

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

class HierarchyWindow : public Object
{
    URHO3D_OBJECT(HierarchyWindow, Object);

public:
    HierarchyWindow(Context* context);
    void SetScene(Scene* scene);

private:
    void CreateWidgets(GenericUIHost* host);
    GenericHierarchyListItem* FindItem(Serializable* object);
    void AddNode(Node* node);

    // @name Scene Events
    // @{

    void HandleNodeAdded(StringHash eventType, VariantMap& eventData);
    void HandleNodeRemoved(StringHash eventType, VariantMap& eventData);
    void HandleComponentAdded(StringHash eventType, VariantMap& eventData);
    void HandleComponentRemoved(StringHash eventType, VariantMap& eventData);
    void HandleNodeNameChanged(StringHash eventType, VariantMap& eventData);
    void HandleNodeEnabledChanged(StringHash eventType, VariantMap& eventData);
    void HandleComponentEnabledChanged(StringHash eventType, VariantMap& eventData);

    // @}

    // @name UI Events
    // @{

    void HandleUIElementNameChanged(StringHash eventType, VariantMap& eventData);
    void HandleUIElementVisibilityChanged(StringHash eventType, VariantMap& eventData);
    void HandleUIElementAttributeChanged(StringHash eventType, VariantMap& eventData);
    void HandleUIElementAdded(StringHash eventType, VariantMap& eventData);
    void HandleUIElementRemoved(StringHash eventType, VariantMap& eventData);

    // @}

private:
    SharedPtr<GenericDialog> dialog_;
    GenericHierarchyList* hierarchyList_ = nullptr;
    SharedPtr<Scene> scene_;
    HashMap<WeakPtr<Serializable>, WeakPtr<GenericHierarchyListItem>> objectsToItems_;
};

}

//////////////////////////////////////////////////////////////////////////
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/Text.h>

namespace Urho3D
{

class UrhoDialog : public GenericDialog
{
    URHO3D_OBJECT(UrhoDialog, GenericDialog);

public:
    UrhoDialog(Context* context);
    void SetName(const String& name) override;

protected:
    void OnChildAdded(GenericWidget* widget) override;

private:
    SharedPtr<Window> window_;
    Text* windowTitle_ = nullptr;
};

class UrhoWidget
{
public:
    virtual UIElement* GetWidget() = 0;
};

class UrhoHierarchyList : public GenericHierarchyList, public UrhoWidget
{
    URHO3D_OBJECT(UrhoHierarchyList, GenericHierarchyList);

public:
    UrhoHierarchyList(Context* context);
    virtual UIElement* GetWidget() { return hierarchyList_; }

protected:
    void OnChildAdded(GenericWidget* widget) override;

private:
    SharedPtr<ListView> hierarchyList_;
};

class UrhoHierarchyListItem : public GenericHierarchyListItem, public UrhoWidget
{
    URHO3D_OBJECT(UrhoHierarchyListItem, GenericHierarchyListItem);

public:
    UrhoHierarchyListItem(Context* context);
    void SetParentListView(ListView* listView) { hierarchyList_ = listView; }
    void SetText(const String& text) override { text_->SetText(text); }
    UIElement* GetWidget() override { return text_; }

protected:
    void OnChildAdded(GenericWidget* widget) override;

private:
    SharedPtr<Text> text_;
    ListView* hierarchyList_ = nullptr;

};

class UrhoUIHost : public GenericUIHost
{
    URHO3D_OBJECT(UrhoUIHost, GenericUIHost);

public:
    UrhoUIHost(Context* context) : GenericUIHost(context) { }

protected:
    GenericWidget* CreateWidgetImpl(StringHash type) override;

};

}
