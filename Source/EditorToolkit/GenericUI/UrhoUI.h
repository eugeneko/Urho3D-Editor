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
    UIElement* GetWidget() override { return hierarchyList_; }
    void SelectItem(GenericHierarchyListItem* item) override;
    void DeselectItem(GenericHierarchyListItem* item) override;
    void GetSelection(ItemVector& result) override;

protected:
    void OnChildAdded(GenericWidget* widget) override;

private:
    void HandleItemClicked(StringHash eventType, VariantMap& eventData);

private:
    SharedPtr<ListView> hierarchyList_;
};

class UrhoHierarchyListItem : public GenericHierarchyListItem, public UrhoWidget
{
    URHO3D_OBJECT(UrhoHierarchyListItem, GenericHierarchyListItem);

public:
    class ItemWidget : public Text
    {
    public:
        ItemWidget(Context* context, UrhoHierarchyListItem* item) : Text(context), item_(item) { }
        UrhoHierarchyListItem* GetGenericItem() { return item_; }
    private:
        UrhoHierarchyListItem* item_ = nullptr;;
    };

    UrhoHierarchyListItem(Context* context);
    void SetParentListView(ListView* listView) { hierarchyList_ = listView; }
    void SetText(const String& text) override { text_->SetText(text); }
    UIElement* GetWidget() override { return text_; }

protected:
    void OnChildAdded(GenericWidget* widget) override;

private:
    SharedPtr<ItemWidget> text_;
    ListView* hierarchyList_ = nullptr;

};

class UrhoUI : public Object, public AbstractUI
{
    URHO3D_OBJECT(UrhoUI, Object);

public:
    UrhoUI(Context* context) : Object(context) { }

protected:
    GenericWidget* CreateWidgetImpl(StringHash type) override;

};

}
