#include "AttributeWidget.h"
#include "AttributeWidgetImpl.h"
#include <Urho3D/Core/Attribute.h>

namespace Urho3DEditor
{

AttributeWidget* AttributeWidget::Construct(const Urho3D::AttributeInfo& attribute)
{
    using namespace Urho3D;
    switch (attribute.type_)
    {
    case VAR_STRING:
        return new StringAttributeWidget();
    default:
        return nullptr;
    }
}

AttributeWidget::AttributeWidget(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{

}

}
