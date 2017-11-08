#include "DockView.h"
#include "TabBar.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/UI/UIEvents.h>

namespace Urho3D
{

namespace
{

static const StringHash tabBarElement("DV_TabBar");
static const StringHash containerElement("DV_DockContainer");

TabBar* GetTabBar(UIElement* element)
{
    return static_cast<TabBar*>(element->GetVar(tabBarElement).GetPtr());
}

void SetTabBar(UIElement* element, TabBar* tabBar)
{
    element->SetVar(tabBarElement, tabBar);
}

UIElement* GetDockContainer(UIElement* element)
{
    return static_cast<UIElement*>(element->GetVar(containerElement).GetPtr());
}

void SetDockContainer(UIElement* element, UIElement* container)
{
    element->SetVar(containerElement, container);
}

}

extern const char* UI_CATEGORY;

DockView::DockView(Context* context)
    : UIElement(context)
{
    SetLayout(LM_VERTICAL);

    // Create elements
    splitElements_[0] = CreateChild<SplitView>("DV_Split" + String(0));
    for (int i = 0; i < DL_COUNT; ++i)
    {
        UIElement* container = splitElements_[i]->CreateFirstChild<UIElement>("DV_Container" + String(i));
        container->SetClipChildren(true);
        container->SetLayout(LM_VERTICAL);
        TabBar* tabBar = container->CreateChild<TabBar>("DV_Tab" + String(i));
        tabBar->SetFill(true);
        SetTabBar(container, tabBar);

        containerElements_[i] = container;
        tabBars_[i] = tabBar;

        if (i < DL_COUNT - 1)
            splitElements_[i + 1] = splitElements_[i]->CreateSecondChild<SplitView>("DV_Split" + String(i + 1));
        else
            centerElement_ = splitElements_[i]->CreateSecondChild<UIElement>("DV_Center");
    }

    UpdateDockSplits();
}

void DockView::RegisterObject(Context* context)
{
    context->RegisterFactory<DockView>(UI_CATEGORY);
    URHO3D_COPY_BASE_ATTRIBUTES(UIElement);

}

void DockView::SetDefaultSplitStyle()
{
    for (SplitView* split : splitElements_)
        split->SetDefaultLineStyle();
}

void DockView::SetDefaultTabBarStyle()
{
    for (TabBar* tabBar : tabBars_)
    {
        tabBar->SetStyle("Menu");
        tabBar->SetHoverOffset(IntVector2::ZERO);
    }
}

void DockView::SetPriority(DockLocation first, DockLocation second, DockLocation third, DockLocation forth)
{
    assert(HashSet<int>({ first, second, third, forth }).Size() == DL_COUNT);

    locations_[0] = first;
    locations_[1] = second;
    locations_[2] = third;
    locations_[3] = forth;

    UpdateDockSplits();
}

void DockView::AddDock(DockLocation location, const String& title, UIElement* content)
{
    UIElement* dockContainer = dockContainers_[location];
    TabBar* tabBar = GetTabBar(dockContainer);
    TabButton* titleButton = tabBar->AddTab(title);
    tabBar->SetMaxHeight(tabBar->GetEffectiveMinSize().y_);

    SetTabBar(titleButton, tabBar);
    SetDockContainer(titleButton, dockContainer);

    SubscribeToEvent(titleButton, E_DRAGMOVE,
        [=](StringHash eventType, VariantMap& eventData)
    {
        const IntVector2 screenPos(eventData[DragMove::P_X].GetInt(), eventData[DragMove::P_Y].GetInt());
        TabBar* tabBar = GetTabBar(titleButton);
        UIElement* dockContainer = GetDockContainer(titleButton);
        UIElement* bestDockContainer = FindBestLocation(screenPos);
        if (bestDockContainer != dockContainer)
        {
            if (bestDockContainer)
            {
                SharedPtr<TabButton> titleButtonHolder(titleButton);
                TabBar* bestTabBar = GetTabBar(bestDockContainer);
                tabBar->RemoveTab(titleButton);
                bestTabBar->AddTab(titleButton);
                bestTabBar->SetMaxHeight(bestTabBar->GetEffectiveMinSize().y_);
                SetDockContainer(titleButton, bestDockContainer);
                SetTabBar(titleButton, bestTabBar);
            }
        }
    });
}

UIElement* DockView::FindBestLocation(const IntVector2& position)
{
    for (UIElement* container : containerElements_)
        if (container->IsInside(position, true))
            return container;
    return nullptr;
}

void DockView::UpdateDockSplits()
{
    for (int i = 0; i < DL_COUNT; ++i)
    {
        const DockLocation location = locations_[i];
        SplitView* split = splitElements_[i];
        UIElement* container = containerElements_[i];
        UIElement* nextElement = i < DL_COUNT - 1 ? splitElements_[i + 1].Get() : centerElement_.Get();

        dockContainers_[location] = container;

        if (location == DL_LEFT || location == DL_RIGHT)
            split->SetSplit(SPLIT_VERTICAL);
        else
            split->SetSplit(SPLIT_HORIZONTAL);

        split->SetFirstChild(nullptr);
        split->SetSecondChild(nullptr);

        if (location == DL_LEFT || location == DL_TOP)
        {
            split->SetFixedPosition(offsets_[location], SA_BEGIN);
            split->SetFirstChild(container);
            split->SetSecondChild(nextElement);
        }
        else
        {
            split->SetFirstChild(nextElement);
            split->SetSecondChild(container);
            split->SetFixedPosition(offsets_[location], SA_END);
        }
    }
}

}

