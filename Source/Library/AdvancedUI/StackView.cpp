#include "StackView.h"
#include <Urho3D/Core/Context.h>

namespace Urho3D
{

extern const char* UI_CATEGORY;

StackView::StackView(Context* context)
    : UIElement(context)
{
    SetLayout(LM_VERTICAL);
    filler_ = CreateChild<UIElement>();
    UpdateChildren();
}

void StackView::RegisterObject(Context* context)
{
    context->RegisterFactory<StackView>(UI_CATEGORY);
    URHO3D_COPY_BASE_ATTRIBUTES(UIElement);
}

void StackView::SetFillEmpty(bool fill)
{
    if (fillEmpty_ != fill)
    {
        fillEmpty_ = fill;
        UpdateChildren();
    }
}

void StackView::AddItem(UIElement* child)
{
    child->SetVisible(false);
    AddChild(child);
}

void StackView::RemoveItem(UIElement* child)
{
    assert(child != filler_);

    if (activeChild_ == child)
    {
        activeChild_ = nullptr;
        UpdateChildren();
    }

    RemoveChild(child);
}

void StackView::SwitchToItem(UIElement* child)
{
    assert(!child || child->GetParent() == this);

    if (activeChild_ != child)
    {
        activeChild_ = child;
        UpdateChildren();
    }
}

void StackView::UpdateChildren()
{
    for (UIElement* child : GetChildren())
        child->SetVisible(false);

    if (activeChild_)
        activeChild_->SetVisible(true);
    else if (fillEmpty_)
        filler_->SetVisible(true);
}

}

