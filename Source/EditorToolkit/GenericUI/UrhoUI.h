#pragma once

#include "GenericUI.h"
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
