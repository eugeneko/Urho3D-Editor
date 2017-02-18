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

/// Array of variants.
using VariantArray = QVector<Urho3D::Variant>;

/// Interface of Urho3D::Serializable attribute editor widget.
class AttributeWidget : public QWidget
{
    Q_OBJECT

public:
    /// Construct widget for attribute.
    static AttributeWidget* Create(const Urho3D::AttributeInfo& info, unsigned index, QWidget* parent = nullptr);

    /// Construct.
    AttributeWidget(QWidget* parent = nullptr);
    /// Get variant value. Value of result variant may be partially or fully re-written.
    virtual void GetValue(Urho3D::Variant& result) const = 0;
    /// Set variant value. Return false is variant has wrong type. Empty variant shall always be acceptable.
    virtual bool SetValue(const Urho3D::Variant& value) = 0;
    /// Set variant value by merging multiple values.
    virtual void SetMergedValue(const VariantArray& value) = 0;

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
    static AttributeWidget* Construct(const Urho3D::AttributeInfo& info, QWidget* parent);

private:
    /// Attribute info.
    Urho3D::AttributeInfo info_;
    /// Attribute index.
    unsigned index_ = Urho3D::M_MAX_UNSIGNED;
};

/// Interface of 'solid' attribute that must be entirely defined or undefined.
class SolidAttributeWidget : public AttributeWidget
{
    Q_OBJECT

public:
    /// Construct.
    SolidAttributeWidget(QWidget* parent = nullptr);
    /// Set undefined flag.
    virtual void SetUndefined(bool undefined);
    /// Set variant value by merging multiple values.
    virtual void SetMergedValue(const VariantArray& value) override;
    /// Return if the value is undefined.
    bool IsUndefined() const { return undefined_; }

private:
    /// Whether the value is undefined.
    bool undefined_ = false;
};

}
