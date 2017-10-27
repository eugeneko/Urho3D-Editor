#pragma once

#include <Urho3D/UI/UIElement.h>

namespace Urho3D
{

enum class DockLocation
{
    Left,
    Right,
    Top,
    Bottom,
};

class DockStation : public UIElement
{
    URHO3D_OBJECT(DockStation, UIElement);

public:
    DockStation(Context* context);

    void InsertDock(UIElement* dock, DockLocation location, unsigned index = M_MAX_UNSIGNED);
    void AddDock(UIElement* dock, DockLocation location);

    void OnResize(const IntVector2& newSize, const IntVector2& delta) override;

private:
    void UpdateChildrenLayout();

private:
    PODVector<UIElement*> docks_;
    unsigned childrenLayoutNestingLevel_ = 0;
};

}
