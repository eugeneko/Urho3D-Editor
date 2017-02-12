#pragma once

#include <Urho3D/Core/Variant.h>
#include <Urho3D/Core/Attribute.h>
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
    static AttributeWidget* Create(const Urho3D::AttributeInfo& info, unsigned index);

    /// Construct.
    AttributeWidget(QWidget* parent = nullptr);
    /// Get variant type.
    virtual Urho3D::VariantType GetVariantType() const = 0;
    /// Get variant value. Value of result variant may be partially or fully re-written.
    virtual void GetValue(Urho3D::Variant& result) const = 0;
    /// Set variant value.
    virtual void SetValue(const Urho3D::Variant& value) = 0;
    /// Set variant value by merging multiple values.
    virtual void SetMergedValue(const QVector<Urho3D::Variant>& value) = 0;

    /// Get attribute info.
    const Urho3D::AttributeInfo& GetInfo() const { return info_; }
    /// Get attribute name.
    const Urho3D::String& GetName() const;
    /// Get attribute index.
    unsigned GetIndex() const { return index_; }

signals:
    /// Signals that value has been changed.
    void valueChanged();
    /// Signals that value has been committed.
    void valueCommitted();

private:
    /// Construct derived widget for specified attribute.
    static AttributeWidget* Construct(const Urho3D::AttributeInfo& info);

private:
    /// Attribute info.
    Urho3D::AttributeInfo info_;
    /// Attribute index.
    unsigned index_;
};

}
