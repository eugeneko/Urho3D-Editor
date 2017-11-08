#pragma once

#include <Urho3D/UI/UIElement.h>

namespace Urho3D
{

class StackView : public UIElement
{
    URHO3D_OBJECT(StackView, UIElement);

public:
    StackView(Context* context);

    static void RegisterObject(Context* context);

    void SetFillEmpty(bool fill);

    void AddItem(UIElement* child);

    void RemoveItem(UIElement* child);

    void SwitchToItem(UIElement* child);

private:
    void UpdateChildren();

private:
    UIElement* filler_ = nullptr;
    UIElement* activeChild_ = nullptr;
    bool fillEmpty_ = true;
};

}
