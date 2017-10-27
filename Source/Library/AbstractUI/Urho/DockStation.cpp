#include "DockStation.h"
#include <Urho3D/UI/UIEvents.h>

namespace Urho3D
{

namespace
{

static const StringHash VAR_LOCATION;

DockLocation GetDockLocation(UIElement* element)
{
    return static_cast<DockLocation>(element->GetVar(VAR_LOCATION).GetInt());
}

void SetDockLocation(UIElement* element, DockLocation location)
{
    element->SetVar(VAR_LOCATION, static_cast<int>(location));
}

enum class DockDimension
{
    Horizontal,
    Vertical
};

DockDimension GetDockDimension(DockLocation location)
{
    switch (location)
    {
    case DockLocation::Left:
    case DockLocation::Right:
        return DockDimension::Horizontal;
    case DockLocation::Top:
    case DockLocation::Bottom:
        return DockDimension::Vertical;
    default:
        assert(0);
        return DockDimension::Horizontal;
    }
}

void UpdateDockLocation(UIElement* element, IntRect& clientArea)
{
    const DockLocation location = GetDockLocation(element);
    const DockDimension dim = GetDockDimension(location);

    // Get dock size
    IntVector2 dockSize;
    IntVector2 minDockSize;
    IntVector2 maxDockSize;
    if (dim == DockDimension::Horizontal)
    {
        minDockSize = IntVector2(element->GetMinWidth(), clientArea.Height());
        //maxDockSize = IntVector2(Max(element->GetMinWidth(), Min(element->GetMaxWidth(), clientArea.Width())), clientArea.Height());
        maxDockSize = IntVector2(clientArea.Width(), clientArea.Height());
        dockSize = VectorMax(minDockSize, VectorMin(element->GetSize(), maxDockSize));
    }
    else
    {
        minDockSize = IntVector2(clientArea.Width(), element->GetMinHeight());
        //maxDockSize = IntVector2(clientArea.Width(), Max(Min(element->GetMaxHeight(), clientArea.Height()), element->GetMinHeight()));
        maxDockSize = IntVector2(clientArea.Width(), clientArea.Height());
        dockSize = VectorMax(minDockSize, VectorMin(element->GetSize(), maxDockSize));
    }

    // Get dock position and shrink client area
    IntVector2 dockPosition(clientArea.left_, clientArea.top_);
    switch (location)
    {
    case DockLocation::Left:
        clientArea.left_ += dockSize.x_;
        break;
    case DockLocation::Right:
        dockPosition.x_ += clientArea.Width() - dockSize.x_;
        clientArea.right_ -= dockSize.x_;
        break;
    case DockLocation::Top:
        clientArea.top_ += dockSize.y_;
        break;
    case DockLocation::Bottom:
        dockPosition.y_ += clientArea.Height() - dockSize.y_;
        clientArea.bottom_ -= dockSize.y_;
        break;
    }

    // Compute min dock size
    element->SetPosition(dockPosition);
    element->SetMinSize(minDockSize);
    element->SetMaxSize(maxDockSize);
    element->SetSize(dockSize);
}

}

DockStation::DockStation(Context* context)
    : UIElement(context)
{
    SubscribeToEvent(this, E_LAYOUTUPDATED, [=](StringHash eventType, VariantMap& eventData)
    {
        UpdateChildrenLayout();
    });
}

void DockStation::InsertDock(UIElement* dock, DockLocation location, unsigned index /*= M_MAX_UNSIGNED*/)
{
    index = Min(index, docks_.Size());
    docks_.Insert(index, dock);
    SetDockLocation(dock, location);
    AddChild(dock);

    SubscribeToEvent(dock, E_POSITIONED, [=](StringHash eventType, VariantMap& eventData)
    {
        UpdateChildrenLayout();
    });
    SubscribeToEvent(dock, E_RESIZED, [=](StringHash eventType, VariantMap& eventData)
    {
        UpdateChildrenLayout();
    });
}

void DockStation::AddDock(UIElement* dock, DockLocation location)
{
    InsertDock(dock, location);
}

void DockStation::OnResize(const IntVector2& newSize, const IntVector2& delta)
{
    UpdateChildrenLayout();
}

void DockStation::UpdateChildrenLayout()
{
    if (childrenLayoutNestingLevel_)
        return;

    ++childrenLayoutNestingLevel_;
    IntRect clientRect(IntVector2::ZERO, GetSize());
    for (UIElement* dock : docks_)
        UpdateDockLocation(dock, clientRect);
    --childrenLayoutNestingLevel_;
}

}
