#include "SerializableWidget.h"
#include "AttributeWidget.h"
#include <Urho3D/Scene/Serializable.h>
#include <QGridLayout>
#include <QLabel>

namespace Urho3DEditor
{

SerializableWidget::SerializableWidget(const SerializableVector& serializables, QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , layout_(new QGridLayout(this))
    , serializables_(serializables)
{
    using namespace Urho3D;
    setLayout(layout_);

    // Enumerate attributes
    String serializableType;
    const Vector<AttributeInfo>* attributes = nullptr;
    for (Urho3D::Serializable* serializable : serializables_)
    {
        if (serializable && serializableType.Empty())
        {
            serializableType = serializable->GetTypeName();
            attributes = serializable->GetAttributes();
            break;
        }
    }
    if (!attributes)
        return;

    int row = layout_->rowCount();
    for (const AttributeInfo& attrib : *attributes)
    {
        if (attrib.mode_ & Urho3D::AM_NOEDIT)
            continue;

        layout_->addWidget(new QLabel(Cast(attrib.name_)), row, 0);
        if (AttributeWidget* attribWidget = AttributeWidget::Construct(attrib))
        {
            layout_->addWidget(attribWidget, row, 1);
            attributes_.push_back(attribWidget);
        }
        ++row;
    }
}

}
