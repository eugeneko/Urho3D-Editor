#include "AttributeWidget.h"
#include "AttributeWidgetImpl.h"
#include "../Bridge.h"

namespace Urho3DEditor
{

AttributeWidget* AttributeWidget::Construct(const Urho3D::AttributeInfo& info, QWidget* parent)
{
    using namespace Urho3D;
    switch (info.type_)
    {
    case VAR_STRING:
        return new StringAttributeWidget(parent);
    case VAR_FLOAT:
    case VAR_DOUBLE:
        return new DoubleAttributeWidget(info.type_, parent);
    case VAR_VECTOR2:
    case VAR_VECTOR3:
    case VAR_VECTOR4:
    case VAR_RECT:
    case VAR_COLOR:
        return new FloatVectorAttributeWidget(info.type_, parent);
    case VAR_QUATERNION:
        return new QuaternionAttributeWidget(parent);
    default:
        return nullptr;
    }
}

AttributeWidget* AttributeWidget::Create(const Urho3D::AttributeInfo& info, unsigned index, QWidget* parent /*= nullptr*/)
{
    if (AttributeWidget* widget = Construct(info, parent))
    {
        widget->index_ = index;
        widget->info_ = info;
        return widget;
    }
    return nullptr;
}

AttributeWidget::AttributeWidget(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
}

const Urho3D::String& AttributeWidget::GetName() const
{
    return info_.name_;
}

//////////////////////////////////////////////////////////////////////////
SolidAttributeWidget::SolidAttributeWidget(QWidget* parent /*= nullptr*/)
    : AttributeWidget(parent)
{

}

void SolidAttributeWidget::SetUndefined(bool undefined)
{
    undefined_ = undefined;
}

void SolidAttributeWidget::SetMergedValue(const VariantArray& values)
{
    if (values.empty())
    {
        SetValue(Urho3D::Variant::EMPTY);
        SetUndefined(false);
    }
    else
    {
        bool undefined = false;
        for (int i = 1; i < values.size(); ++i)
            if (values[i] != values[0])
            {
                undefined = true;
                break;
            }

        SetUndefined(undefined);
        SetValue(undefined ? Urho3D::Variant::EMPTY : values[0]);
    }
}

}
