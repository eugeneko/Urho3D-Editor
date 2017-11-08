#include "DockView.h"
#include "TabBar.h"
#include "StackView.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/UI/UIEvents.h>

namespace Urho3D
{

namespace
{

static const StringHash panelTabBarElement("DV_PanelTabBar");
static const StringHash panelElement("DV_Panel");
static const StringHash panelStackElement("DV_PanelStack");
static const StringHash contentElement("DV_Content");

TabBar* GetPanelTabBar(UIElement* element)
{
    return static_cast<TabBar*>(element->GetVar(panelTabBarElement).GetPtr());
}

void SetPanelTabBar(UIElement* element, TabBar* tabBar)
{
    element->SetVar(panelTabBarElement, tabBar);
}

UIElement* GetPanel(UIElement* element)
{
    return static_cast<UIElement*>(element->GetVar(panelElement).GetPtr());
}

void SetPanel(UIElement* element, UIElement* panel)
{
    element->SetVar(panelElement, panel);
}

StackView* GetPanelStack(UIElement* element)
{
    return static_cast<StackView*>(element->GetVar(panelStackElement).GetPtr());
}

void SetPanelStack(UIElement* element, StackView* stack)
{
    element->SetVar(panelStackElement, stack);
}

UIElement* GetContent(UIElement* element)
{
    return static_cast<UIElement*>(element->GetVar(contentElement).GetPtr());
}

void SetContent(UIElement* element, UIElement* content)
{
    element->SetVar(contentElement, content);
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

        StackView* stack = container->CreateChild<StackView>("DV_Stack" + String(i));
        SetPanelTabBar(container, tabBar);
        SetPanelStack(container, stack);

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
    UIElement* panel = dockContainers_[location];
    TabBar* tabBar = GetPanelTabBar(panel);
    TabButton* titleButton = tabBar->AddTab(title);
    tabBar->SetMaxHeight(tabBar->GetEffectiveMinSize().y_);

    SetContent(titleButton, content);
    RelocateDock(titleButton, panel, IntVector2::ZERO);

    SubscribeToEvent(titleButton, E_DRAGMOVE,
        [=](StringHash eventType, VariantMap& eventData)
    {
        const IntVector2 screenPos(eventData[DragMove::P_X].GetInt(), eventData[DragMove::P_Y].GetInt());
        TabBar* tabBar = GetPanelTabBar(titleButton);
        UIElement* panel = GetPanel(titleButton);
        UIElement* bestPanel = FindBestLocation(screenPos);
        if (bestPanel && bestPanel != panel)
        {
            RelocateDock(titleButton, bestPanel, screenPos);
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

void DockView::RelocateDock(TabButton* dockTitle, UIElement* newPanel, const IntVector2& hintPosition)
{
    // Get all related elements
    UIElement* dockContent = GetContent(dockTitle);
    UIElement* oldPanel = GetPanel(dockTitle);

    // Keep title and content
    SharedPtr<TabButton> dockTitleHolder(dockTitle);
    SharedPtr<UIElement> dockContentHolder(dockContent);

    // Remove dock from old panel
    if (oldPanel)
    {
        TabBar* oldTabBar = GetPanelTabBar(oldPanel);
        StackView* oldStack = GetPanelStack(oldPanel);

        assert(oldTabBar == GetPanelTabBar(dockTitle));
        assert(oldStack == GetPanelStack(dockTitle));

        oldTabBar->RemoveTab(dockTitle);
        oldStack->RemoveItem(dockContent);

        // Reset variables
        SetPanel(dockTitle, nullptr);
        SetPanelTabBar(dockTitle, nullptr);
        SetPanelStack(dockTitle, nullptr);
    }

    // Put dock into new panel
    if (newPanel)
    {
        TabBar* newTabBar = GetPanelTabBar(newPanel);
        StackView* newStack = GetPanelStack(newPanel);

        newTabBar->AddTab(dockTitle);
        newStack->AddItem(dockContent);

        // Set variables
        SetPanel(dockTitle, newPanel);
        SetPanelTabBar(dockTitle, newTabBar);
        SetPanelStack(dockTitle, newStack);
    }
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

