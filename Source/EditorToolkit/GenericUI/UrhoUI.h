#pragma once

#include "GenericUI.h"
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/Text.h>

namespace Urho3D
{

class UrhoUI;

class UrhoDialog : public GenericDialog
{
    URHO3D_OBJECT(UrhoDialog, GenericDialog);

public:
    UrhoDialog(AbstractUI& ui, GenericWidget* parent);
    void SetBodyWidget(GenericWidget* widget) override;
    void SetName(const String& name) override;

private:
    SharedPtr<Window> window_;
    Text* windowTitle_ = nullptr;
    SharedPtr<GenericWidget> body_;
    UIElement* bodyElement_ = nullptr;
};

class UrhoWidget
{
public:
    virtual UIElement* GetWidget() = 0;
};

class UrhoHierarchyListItemWidget : public Text
{
public:
    UrhoHierarchyListItemWidget(Context* context, GenericHierarchyListItem* item);
    GenericHierarchyListItem* GetItem() { return item_; }
private:
    GenericHierarchyListItem* item_ = nullptr;;
};

class UrhoHierarchyList : public GenericHierarchyList, public UrhoWidget
{
    URHO3D_OBJECT(UrhoHierarchyList, GenericHierarchyList);

public:
    UrhoHierarchyList(AbstractUI& ui, GenericWidget* parent);
    UIElement* GetWidget() override { return hierarchyList_; }
    void AddItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent) override;
    void SelectItem(GenericHierarchyListItem* item) override;
    void DeselectItem(GenericHierarchyListItem* item) override;
    void GetSelection(ItemVector& result) override;

private:
    void InsertItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent);
    void HandleItemClicked(StringHash eventType, VariantMap& eventData);

private:
    SharedPtr<ListView> hierarchyList_;
    GenericHierarchyListItem rootItem_;
};

class UrhoMainWindow : public GenericMainWindow, public Object
{
    URHO3D_OBJECT(UrhoMainWindow, Object);

public:
    UrhoMainWindow(Context* context, UrhoUI& ui) : GenericMainWindow(), Object(context), ui_(ui) { }
    virtual GenericDialog* AddDialog(DialogLocationHint hint = DialogLocationHint::Undocked) override;

private:
    UrhoUI& ui_;
    Vector<SharedPtr<UrhoDialog>> dialogs_;
};

class UrhoUI : public Object, public AbstractUI
{
    URHO3D_OBJECT(UrhoUI, Object);

public:
    UrhoUI(Context* context) : Object(context), mainWindow_(context, *this) { }
    GenericWidget* CreateWidget(StringHash type, GenericWidget* parent) override;
    Context* GetContext() override { return Object::GetContext(); }
    GenericMainWindow* GetMainWindow() override { return &mainWindow_; }

private:
    UrhoMainWindow mainWindow_;

};

}
