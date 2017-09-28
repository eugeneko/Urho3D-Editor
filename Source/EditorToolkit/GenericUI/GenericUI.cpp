#include "GenericUI.h"
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>

//////////////////////////////////////////////////////////////////////////
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/LineEdit.h>

namespace Urho3D
{

GenericWidget* GenericWidget::CreateChild(StringHash type)
{
    SharedPtr<GenericWidget> child(host_->CreateWidget(type));
    children_.Push(child);
    OnChildAdded(child);
    return child;
}

void GenericWidget::OnChildAdded(GenericWidget* widget)
{
}

//////////////////////////////////////////////////////////////////////////
GenericWidget* GenericUIHost::CreateWidget(StringHash type)
{
    GenericWidget* widget = CreateWidgetImpl(type);
    if (widget)
        widget->SetHost(this);
    return widget;
}

//////////////////////////////////////////////////////////////////////////
HierarchyWindow::HierarchyWindow(Context* context) : Object(context)
{
    CreateWidgets(GetSubsystem<GenericUI>()->GetDefaultHost());
}

void HierarchyWindow::SetScene(Scene* scene)
{
    if (scene_)
    {
        UnsubscribeFromEvent(scene_, E_NODEADDED);
        UnsubscribeFromEvent(scene_, E_NODEREMOVED);
        UnsubscribeFromEvent(scene_, E_COMPONENTADDED);
        UnsubscribeFromEvent(scene_, E_COMPONENTREMOVED);
        UnsubscribeFromEvent(scene_, E_NODENAMECHANGED);
        UnsubscribeFromEvent(scene_, E_NODEENABLEDCHANGED);
        UnsubscribeFromEvent(scene_, E_COMPONENTENABLEDCHANGED);
        //unsigned int index = GetListIndex(scene_);
        //UpdateHierarchyItem(index, NULL, NULL);
    }
    scene_ = scene;
    if (scene_)
    {
        //UpdateHierarchyItem(scene_);
        SubscribeToEvent(scene_, E_NODEADDED, URHO3D_HANDLER(HierarchyWindow, HandleNodeAdded));
        SubscribeToEvent(scene_, E_NODEREMOVED, URHO3D_HANDLER(HierarchyWindow, HandleNodeRemoved));
        SubscribeToEvent(scene_, E_COMPONENTADDED, URHO3D_HANDLER(HierarchyWindow, HandleComponentAdded));
        SubscribeToEvent(scene_, E_COMPONENTREMOVED, URHO3D_HANDLER(HierarchyWindow, HandleComponentRemoved));
        SubscribeToEvent(scene_, E_NODENAMECHANGED, URHO3D_HANDLER(HierarchyWindow, HandleNodeNameChanged));
        SubscribeToEvent(scene_, E_NODEENABLEDCHANGED, URHO3D_HANDLER(HierarchyWindow, HandleNodeEnabledChanged));
        SubscribeToEvent(scene_, E_COMPONENTENABLEDCHANGED, URHO3D_HANDLER(HierarchyWindow, HandleComponentEnabledChanged));
        AddNode(scene_);
    }
}

void HierarchyWindow::CreateWidgets(GenericUIHost* host)
{
    dialog_ = host->CreateWidget<GenericDialog>();
    dialog_->SetName("Hierarchy");

    hierarchyList_ = dialog_->CreateChild<GenericHierarchyList>();
    SetScene(scene_);
}

GenericHierarchyListItem* HierarchyWindow::FindItem(Serializable* object)
{
    if (object)
        return objectsToItems_[WeakPtr<Serializable>(object)];
    else
        return nullptr;
}

void HierarchyWindow::AddNode(Node* node)
{
    Node* parent = node->GetParent();
    GenericHierarchyListItem* parentItem = FindItem(parent);
    GenericHierarchyListItem* objectItem = parentItem
        ? parentItem->CreateChild<GenericHierarchyListItem>()
        : hierarchyList_->CreateChild<GenericHierarchyListItem>();

    objectsToItems_[WeakPtr<Serializable>(node)] = objectItem;

    if (Scene* scene = dynamic_cast<Scene*>(node))
        objectItem->SetText(scene->GetName().Empty() ? "Scene" : scene->GetName());
    else
        objectItem->SetText(node->GetName().Empty() ? "Node" : node->GetName());

    for (Node* child : node->GetChildren())
        AddNode(child);
}

void HierarchyWindow::HandleNodeAdded(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleNodeRemoved(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleComponentAdded(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleComponentRemoved(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleNodeNameChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleNodeEnabledChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleComponentEnabledChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleUIElementNameChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleUIElementVisibilityChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleUIElementAttributeChanged(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleUIElementAdded(StringHash eventType, VariantMap& eventData)
{

}

void HierarchyWindow::HandleUIElementRemoved(StringHash eventType, VariantMap& eventData)
{

}

//////////////////////////////////////////////////////////////////////////
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
}

void UrhoHierarchyList::OnChildAdded(GenericWidget* widget)
{
    if (auto urhoWidget = dynamic_cast<UrhoWidget*>(widget))
        hierarchyList_->InsertItem(M_MAX_UNSIGNED, urhoWidget->GetWidget());
    if (auto listItem = dynamic_cast<UrhoHierarchyListItem*>(widget))
        listItem->SetParentListView(hierarchyList_);
}

//////////////////////////////////////////////////////////////////////////
UrhoHierarchyListItem::UrhoHierarchyListItem(Context* context)
    : GenericHierarchyListItem(context)
{
    text_ = MakeShared<Text>(context);
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
GenericWidget* UrhoUIHost::CreateWidgetImpl(StringHash type)
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
