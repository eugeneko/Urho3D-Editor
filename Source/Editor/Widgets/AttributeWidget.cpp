#include "AttributeWidget.h"
#include "AttributeWidgetImpl.h"
#include "../Bridge.h"

namespace Urho3DEditor
{

AttributeWidget* AttributeWidget::Construct(const Urho3D::AttributeInfo& info)
{
    using namespace Urho3D;
    switch (info.type_)
    {
    case VAR_STRING:
        return new StringAttributeWidget();
    default:
        return nullptr;
    }
}

AttributeWidget* AttributeWidget::Create(const Urho3D::AttributeInfo& info, unsigned index)
{
    if (AttributeWidget* widget = Construct(info))
    {
        widget->info_ = info;
        widget->index_ = index;
        return widget;
    }
    return nullptr;
}

AttributeWidget::AttributeWidget(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , index_(0)
{

}

const Urho3D::String& AttributeWidget::GetName() const
{
    return info_.name_;
}

}
