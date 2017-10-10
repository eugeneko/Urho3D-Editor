#include "SerializableWidget.h"
#include "AttributeWidget.h"
#include <Urho3D/Scene/Serializable.h>
#include <QGridLayout>
#include <QLabel>

namespace Urho3DEditor
{

const Urho3D::Vector<Urho3D::AttributeInfo> SerializableWidget::emptyAttribArray;

SerializableWidget::SerializableWidget(const SerializableVector& serializables, QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , layout_(new QGridLayout(this))
    , attributes_(&emptyAttribArray)
{
    using namespace Urho3D;
    setLayout(layout_);

    // Enumerate attributes
    for (Urho3D::Serializable* serializable : serializables)
    {
        assert(serializable);

        if (serializableType_.Empty())
        {
            serializableType_ = serializable->GetTypeName();
            attributes_ = serializable->GetAttributes();
        }
        if (serializable->GetTypeName() == serializableType_)
            serializables_.push_back(serializable);
    }

    // Create widgets
    for (unsigned i = 0; i < attributes_->Size(); ++i)
    {
        const AttributeInfo& attrib = attributes_->At(i);
        if (attrib.mode_ & Urho3D::AM_NOEDIT)
            continue;

        const int row = layout_->rowCount();
        QLabel* lbl = nullptr;
        layout_->addWidget(lbl = new QLabel(Cast(attrib.name_)), row, 0);
        lbl->setMinimumWidth(10);
        if (AttributeWidget* attribWidget = AttributeWidget::Create(attrib, i))
        {
            layout_->addWidget(attribWidget, row, 1);
            attributeWidgets_.push_back(attribWidget);
            connect(attribWidget, &AttributeWidget::valueChanged, this, &SerializableWidget::HandleAttributeChanged);
            connect(attribWidget, &AttributeWidget::valueCommitted, this, &SerializableWidget::HandleAttributeCommitted);
        }
    }

    // Setup values
    Update();
}

void SerializableWidget::Update()
{
    for (AttributeWidget* widget : attributeWidgets_)
        widget->SetMergedValue(GatherAttribute(widget->GetIndex()));
}

void SerializableWidget::HandleAttributeChanged()
{
    AttributeWidget* attribWidget = qobject_cast<AttributeWidget*>(sender());
    if (!attribWidget || serializables_.empty())
        return;

    const unsigned attribIndex = attribWidget->GetIndex();
    QVector<Urho3D::Variant> newValues;
    for (Urho3D::Serializable* serializable : serializables_)
    {
        Urho3D::Variant oldValue = serializable->GetAttribute(attribIndex);
        attribWidget->GetValue(oldValue);
        newValues.push_back(oldValue);
    }

    emit attributeChanged(serializables_, attribIndex, newValues);
}

void SerializableWidget::HandleAttributeCommitted()
{
    AttributeWidget* attribWidget = qobject_cast<AttributeWidget*>(sender());
    if (!attribWidget || serializables_.empty())
        return;

    const unsigned attribIndex = attribWidget->GetIndex();
    emit attributeCommitted(serializables_, attribIndex);
}

QVector<Urho3D::Variant> SerializableWidget::GatherAttribute(unsigned attribIndex) const
{
    QVector<Urho3D::Variant> result;
    result.resize(serializables_.size());
    for (int i = 0; i < serializables_.size(); ++i)
        result[i] = serializables_[i]->GetAttribute(attribIndex);
    return result;
}

}
