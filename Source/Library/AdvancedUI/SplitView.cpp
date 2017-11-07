#include "SplitView.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>

namespace Urho3D
{

extern const char* UI_CATEGORY;

SplitLine::SplitLine(Context* context)
    : BorderImage(context)
{
    SetEnabled(true);
}

void SplitLine::RegisterObject(Context* context)
{
    context->RegisterFactory<SplitLine>(UI_CATEGORY);
    URHO3D_COPY_BASE_ATTRIBUTES(BorderImage);
}

void SplitLine::SetOrientation(SplitOrientation orientation)
{
    orientation_ = orientation;
}

void SplitLine::OnHover(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor)
{
    UIElement::OnHover(position, screenPosition, buttons, qualifiers, cursor);

    UI* ui = GetSubsystem<UI>();
    if (Cursor* cursor = ui->GetCursor())
        cursor->SetShape(orientation_ == SPLIT_VERTICAL ? CS_RESIZEHORIZONTAL : CS_RESIZEVERTICAL);
}

void SplitLine::OnDragBegin(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor)
{
    UIElement::OnDragBegin(position, screenPosition, buttons, qualifiers, cursor);

    if (buttons == MOUSEB_LEFT)
        dragBeginPosition_ = GetPosition();
}

void SplitLine::OnDragMove(const IntVector2& /*position*/, const IntVector2& /*screenPosition*/, const IntVector2& deltaPos, int buttons, int /*qualifiers*/, Cursor* /*cursor*/)
{
    if (buttons == MOUSEB_LEFT)
    {
        IntVector2 position = GetPosition();
        if (orientation_ == SPLIT_VERTICAL)
            position.x_ = Clamp(position.x_ + deltaPos.x_, 0, GetParent()->GetWidth() - threshold_);
        else
            position.y_ = Clamp(position.y_ + deltaPos.y_, 0, GetParent()->GetHeight() - threshold_);
        SetPosition(position);
    }
}

void SplitLine::OnDragEnd(const IntVector2& position, const IntVector2& screenPosition, int dragButtons, int buttons, Cursor* cursor)
{
    UIElement::OnDragEnd(position, screenPosition, dragButtons, buttons, cursor);
}

void SplitLine::OnDragCancel(const IntVector2& position, const IntVector2& screenPosition, int dragButtons, int buttons, Cursor* cursor)
{
    UIElement::OnDragCancel(position, screenPosition, dragButtons, buttons, cursor);

    if (dragButtons == MOUSEB_LEFT)
        SetPosition(dragBeginPosition_);
}

//////////////////////////////////////////////////////////////////////////
SplitView::SplitView(Context* context)
    : UIElement(context)
{
    splitLine_ = CreateChild<SplitLine>("SV_SplitLine");

    SubscribeToEvent(splitLine_, E_DRAGMOVE, URHO3D_HANDLER(SplitView, HandleSplitLineMoved));

    SetThreshold(8);
}

void SplitView::RegisterObject(Context* context)
{
    context->RegisterFactory<SplitView>(UI_CATEGORY);
    URHO3D_COPY_BASE_ATTRIBUTES(UIElement);
}

void SplitView::SetDefaultLineStyle()
{
    splitLine_->SetStyle("Menu");
    splitLine_->SetHoverOffset(IntVector2::ZERO);
}

void SplitView::SetThreshold(int threshold)
{
    threshold_ = threshold;
    splitLine_->SetThreshold(threshold);
    UpdateSplitLine();
    UpdateChildren();
}

void SplitView::SetSplit(SplitOrientation orientation)
{
    orientation_ = orientation;
    splitLine_->SetOrientation(orientation);
    UpdateSplitLine();
    UpdateChildren();
}

void SplitView::SetRelativePosition(float relativePosition)
{
    useRelativePosition_ = true;
    relativePosition_ = relativePosition;
    UpdateSplitLine();
    UpdateChildren();
}

void SplitView::SetFixedPosition(int offset, SplitAnchor anchor)
{
    useRelativePosition_ = false;
    offset_ = offset;
    anchor_ = anchor;
}

void SplitView::SetFirstChild(UIElement* child)
{
    if (firstChild_)
        RemoveChild(firstChild_);
    firstChild_ = child;
    AddChild(firstChild_);

    UpdateChildren();
}

void SplitView::SetSecondChild(UIElement* child)
{
    if (secondChild_)
        RemoveChild(secondChild_);
    secondChild_ = child;
    AddChild(secondChild_);

    UpdateChildren();
}

UIElement* SplitView::CreateFirstChild(StringHash type, const String& name /*= String::EMPTY*/)
{
    SharedPtr<UIElement> newElement = DynamicCast<UIElement>(context_->CreateObject(type));
    if (!newElement)
    {
        URHO3D_LOGERROR("Could not create unknown UI element type " + type.ToString());
        return nullptr;
    }

    if (!name.Empty())
        newElement->SetName(name);

    SetFirstChild(newElement);
    return newElement;
}

UIElement* SplitView::CreateSecondChild(StringHash type, const String& name /*= String::EMPTY*/)
{
    SharedPtr<UIElement> newElement = DynamicCast<UIElement>(context_->CreateObject(type));
    if (!newElement)
    {
        URHO3D_LOGERROR("Could not create unknown UI element type " + type.ToString());
        return nullptr;
    }

    if (!name.Empty())
        newElement->SetName(name);

    SetSecondChild(newElement);
    return newElement;
}

void SplitView::OnResize(const IntVector2& newSize, const IntVector2& delta)
{
    UpdateSplitLine();
    UpdateChildren();
}

void SplitView::HandleSplitLineMoved(StringHash eventType, VariantMap& eventData)
{
    if (orientation_ == SPLIT_VERTICAL)
    {
        const int position = ElementToLinePosition(splitLine_->GetPosition().x_);
        if (anchor_ == SA_BEGIN)
            offset_ = position;
        else
            offset_ = GetWidth() - 1 - position;
        relativePosition_ = static_cast<float>(position) / GetWidth();
    }
    else
    {
        const int position = ElementToLinePosition(splitLine_->GetPosition().y_);
        if (anchor_ == SA_BEGIN)
            offset_ = position;
        else
            offset_ = GetHeight() - 1 - position;
        relativePosition_ = static_cast<float>(position) / GetHeight();
    }

    UpdateChildren();
}

void SplitView::UpdateSplitLine()
{
    if (orientation_ == SPLIT_VERTICAL)
    {
        if (useRelativePosition_)
        {
            const int position = static_cast<int>(GetWidth() * relativePosition_);
            splitLine_->SetPosition(LineToElementPosition(position, GetWidth()), 0);
        }
        else if (anchor_ == SA_BEGIN)
            splitLine_->SetPosition(LineToElementPosition(offset_, GetWidth()), 0);
        else
            splitLine_->SetPosition(LineToElementPosition(GetWidth() - 1 - offset_, GetWidth()), 0);

        splitLine_->SetFixedSize(threshold_, GetHeight());
    }
    else
    {
        if (useRelativePosition_)
        {
            const int position = static_cast<int>(GetHeight() * relativePosition_);
            splitLine_->SetPosition(0, LineToElementPosition(position, GetHeight()));
        }
        else if (anchor_ == SA_BEGIN)
            splitLine_->SetPosition(0, LineToElementPosition(offset_, GetHeight()));
        else
            splitLine_->SetPosition(0, LineToElementPosition(GetHeight() - 1 - offset_, GetHeight()));

        splitLine_->SetFixedSize(GetWidth(), threshold_);
    }
}

void SplitView::UpdateChildren()
{
    if (orientation_ == SPLIT_VERTICAL)
    {
        const int splitPosition = splitLine_->GetPosition().x_;
        if (firstChild_)
        {
            firstChild_->SetPosition(0, 0);
            firstChild_->SetSize(splitPosition, GetHeight());
        }
        if (secondChild_)
        {
            secondChild_->SetPosition(splitPosition + threshold_, 0);
            secondChild_->SetSize(GetWidth() - splitPosition - threshold_, GetHeight());
        }
    }
    else
    {
        const int splitPosition = splitLine_->GetPosition().y_;
        if (firstChild_)
        {
            firstChild_->SetPosition(0, 0);
            firstChild_->SetSize(GetWidth(), splitPosition);
        }
        if (secondChild_)
        {
            secondChild_->SetPosition(0, splitPosition + threshold_);
            secondChild_->SetSize(GetWidth(), GetHeight() - splitPosition - threshold_);
        }
    }
}

}
