#pragma once

#include <Urho3D/Core/Variant.h>
#include <QWidget>

namespace Urho3D
{

struct AttributeInfo;

}

namespace Urho3DEditor
{

/// Interface of Urho3D::Serializable attribute editor widget.
class AttributeWidget : public QWidget
{
    Q_OBJECT

public:
    /// Construct widget for attribute.
    static AttributeWidget* Construct(const Urho3D::AttributeInfo& attribute);

    /// Construct.
    AttributeWidget(QWidget* parent = nullptr);
    /// Get variant type.
    virtual Urho3D::VariantType GetVariantType() const = 0;
    /// Get variant value. Value of result variant may be partially or fully re-written.
    virtual void GetValue(Urho3D::Variant& result) const = 0;
    /// Set variant value.
    virtual void SetValue(const Urho3D::Variant& value) = 0;

signals:
    /// Signals that value has changed.
    void valueChanged();
};

}
