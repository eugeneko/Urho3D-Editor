#include "UrhoUI.h"
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/LineEdit.h>

namespace Urho3D
{

UrhoDialog::UrhoDialog(Context* context)
    : GenericDialog(context)
{
    UI* ui = GetSubsystem<UI>();
    UIElement* uiRoot = ui->GetRoot();

    // Create window
    window_ = uiRoot->CreateChild<Window>();
    window_->SetStyleAuto();
    window_->SetMinWidth(200);
    window_->SetMinHeight(200);
    window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    window_->SetAlignment(HA_CENTER, VA_CENTER);
    window_->SetName("Window");

    // Create title
    UIElement* titleBar = window_->CreateChild<UIElement>();
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    windowTitle_ = titleBar->CreateChild<Text>();
    windowTitle_->SetStyleAuto();
    windowTitle_->SetName("WindowTitle");

    Button* buttonClose = titleBar->CreateChild<Button>();
    buttonClose->SetStyle("CloseButton");
    buttonClose->SetName("CloseButton");

    titleBar->SetFixedHeight(titleBar->GetMinHeight());
}

void UrhoDialog::SetName(const String& name)
{
    windowTitle_->SetText(name);
}

void UrhoDialog::OnChildAdded(GenericWidget* widget)
{
    if (auto urhoWidget = dynamic_cast<UrhoWidget*>(widget))
        window_->AddChild(urhoWidget->GetWidget());
}

//////////////////////////////////////////////////////////////////////////
UrhoHierarchyList::UrhoHierarchyList(Context* context)
    : GenericHierarchyList(context)
{
    hierarchyList_ = new ListView(context_);
    hierarchyList_->SetInternal(true);
    hierarchyList_->SetName("HierarchyList");
    hierarchyList_->SetHighlightMode(HM_ALWAYS);
    hierarchyList_->SetMultiselect(true);
    hierarchyList_->SetSelectOnClickEnd(true);
    hierarchyList_->SetHierarchyMode(true);
    hierarchyList_->SetStyle("HierarchyListView");
    SubscribeToEvent(hierarchyList_, E_ITEMCLICKED, URHO3D_HANDLER(UrhoHierarchyList, HandleItemClicked));
}

void UrhoHierarchyList::SelectItem(GenericHierarchyListItem* item)
{
    if (auto urhoItem = dynamic_cast<UrhoHierarchyListItem*>(item))
    {
        const unsigned index = hierarchyList_->FindItem(urhoItem->GetWidget());
        if (!hierarchyList_->IsSelected(index))
            hierarchyList_->ToggleSelection(index);
    }
}

void UrhoHierarchyList::DeselectItem(GenericHierarchyListItem* item)
{
    if (auto urhoItem = dynamic_cast<UrhoHierarchyListItem*>(item))
    {
        const unsigned index = hierarchyList_->FindItem(urhoItem->GetWidget());
        if (hierarchyList_->IsSelected(index))
            hierarchyList_->ToggleSelection(index);
    }
}

void UrhoHierarchyList::GetSelection(ItemVector& result)
{
    for (unsigned index : hierarchyList_->GetSelections())
    {
        UIElement* element = hierarchyList_->GetItem(index);
        if (auto item = dynamic_cast<UrhoHierarchyListItem::ItemWidget*>(element))
            result.Push(item->GetGenericItem());
    }
}

void UrhoHierarchyList::OnChildAdded(GenericWidget* widget)
{
    if (auto urhoWidget = dynamic_cast<UrhoWidget*>(widget))
        hierarchyList_->InsertItem(M_MAX_UNSIGNED, urhoWidget->GetWidget());
    if (auto listItem = dynamic_cast<UrhoHierarchyListItem*>(widget))
        listItem->SetParentListView(hierarchyList_);
}

void UrhoHierarchyList::HandleItemClicked(StringHash /*eventType*/, VariantMap& eventData)
{
    if (auto item = dynamic_cast<UrhoHierarchyListItem::ItemWidget*>(eventData[ItemClicked::P_ITEM].GetPtr()))
    {
        SendEvent(E_GENERICWIDGETCLICKED,
            GenericWidgetClicked::P_ELEMENT, this,
            GenericWidgetClicked::P_ITEM, item->GetGenericItem());
    }
}

//////////////////////////////////////////////////////////////////////////
UrhoHierarchyListItem::UrhoHierarchyListItem(Context* context)
    : GenericHierarchyListItem(context)
{
    text_ = MakeShared<ItemWidget>(context_, this);
    text_->SetStyle("FileSelectorListText");
}

void UrhoHierarchyListItem::OnChildAdded(GenericWidget* widget)
{
    if (auto urhoWidget = dynamic_cast<UrhoWidget*>(widget))
        hierarchyList_->InsertItem(M_MAX_UNSIGNED, urhoWidget->GetWidget(), GetWidget());
    if (auto listItem = dynamic_cast<UrhoHierarchyListItem*>(widget))
        listItem->SetParentListView(hierarchyList_);
}

//////////////////////////////////////////////////////////////////////////
GenericWidget* UrhoUI::CreateWidgetImpl(StringHash type)
{
    if (type == GenericDialog::GetTypeStatic())
        return new UrhoDialog(context_);
    else if (type == GenericHierarchyList::GetTypeStatic())
        return new UrhoHierarchyList(context_);
    else if (type == GenericHierarchyListItem::GetTypeStatic())
        return new UrhoHierarchyListItem(context_);
    return nullptr;
}

}
