#include "UrhoUI.h"
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/LineEdit.h>

namespace Urho3D
{

UrhoDialog::UrhoDialog(AbstractUI& ui, GenericWidget* parent)
    : GenericDialog(ui, parent)
{
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
}

void UrhoDialog::SetBodyWidget(GenericWidget* widget)
{
    if (bodyElement_)
    {
        window_->RemoveChild(bodyElement_);
        body_ = nullptr;
        bodyElement_ = nullptr;
    }
    if (auto urhoWidget = dynamic_cast<UrhoWidget*>(widget))
    {
        body_ = widget;
        bodyElement_ = urhoWidget->GetWidget();
        window_->AddChild(bodyElement_);
    }
}

void UrhoDialog::SetName(const String& name)
{
    windowTitle_->SetText(name);
}

//////////////////////////////////////////////////////////////////////////
UrhoHierarchyList::UrhoHierarchyList(AbstractUI& ui, GenericWidget* parent)
    : GenericHierarchyList(ui, parent)
    , rootItem_(context_)
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

void UrhoHierarchyList::AddItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent)
{
    hierarchyList_->DisableInternalLayoutUpdate();
    if (parent)
        parent->InsertChild(item, index);
    else
        rootItem_.InsertChild(item, index);
    InsertItem(item, index, parent);
    hierarchyList_->EnableInternalLayoutUpdate();
    hierarchyList_->UpdateInternalLayout();
}

void UrhoHierarchyList::SelectItem(GenericHierarchyListItem* item)
{
    if (auto itemWidget = dynamic_cast<UIElement*>(item->GetInternalPointer()))
    {
        const unsigned index = hierarchyList_->FindItem(itemWidget);
        if (!hierarchyList_->IsSelected(index))
            hierarchyList_->ToggleSelection(index);
    }
}

void UrhoHierarchyList::DeselectItem(GenericHierarchyListItem* item)
{
    if (auto itemWidget = dynamic_cast<UIElement*>(item->GetInternalPointer()))
    {
        const unsigned index = hierarchyList_->FindItem(itemWidget);
        if (hierarchyList_->IsSelected(index))
            hierarchyList_->ToggleSelection(index);
    }
}

void UrhoHierarchyList::GetSelection(ItemVector& result)
{
    for (unsigned index : hierarchyList_->GetSelections())
    {
        UIElement* element = hierarchyList_->GetItem(index);
        if (auto item = dynamic_cast<UrhoHierarchyListItemWidget*>(element))
            result.Push(item->GetItem());
    }
}

void UrhoHierarchyList::InsertItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent)
{
    auto itemWidget = MakeShared<UrhoHierarchyListItemWidget>(context_, item);
    itemWidget->SetText(item->GetText());
    item->SetInternalPointer(itemWidget);

    UIElement* parentWidget = parent ? dynamic_cast<UIElement*>(parent->GetInternalPointer()) : nullptr;
    if (itemWidget)
    {
        hierarchyList_->InsertItem(M_MAX_UNSIGNED, itemWidget, parentWidget);
        for (unsigned i = 0; i < item->GetNumChildren(); ++i)
            InsertItem(item->GetChild(i), M_MAX_UNSIGNED, item);
    }
}

void UrhoHierarchyList::HandleItemClicked(StringHash /*eventType*/, VariantMap& eventData)
{
    RefCounted* element = eventData[ItemClicked::P_ITEM].GetPtr();
    if (auto item = dynamic_cast<UrhoHierarchyListItemWidget*>(element))
    {
        SendEvent(E_GENERICWIDGETCLICKED,
            GenericWidgetClicked::P_ELEMENT, this,
            GenericWidgetClicked::P_ITEM, item->GetItem());
    }
}

//////////////////////////////////////////////////////////////////////////
UrhoHierarchyListItemWidget::UrhoHierarchyListItemWidget(Context* context, GenericHierarchyListItem* item)
    : Text(context)
    , item_(item)
{
    SetStyle("FileSelectorListText");
}

//////////////////////////////////////////////////////////////////////////
GenericDialog* UrhoMainWindow::AddDialog(DialogLocationHint hint)
{
    dialogs_.Push(MakeShared<UrhoDialog>(ui_, nullptr));
    return dialogs_.Back();
}

//////////////////////////////////////////////////////////////////////////
GenericWidget* UrhoUI::CreateWidget(StringHash type, GenericWidget* parent)
{
    GenericWidget* widget = nullptr;
    if (type == GenericHierarchyList::GetTypeStatic())
        widget = new UrhoHierarchyList(*this, parent);
    return widget;
}

}
