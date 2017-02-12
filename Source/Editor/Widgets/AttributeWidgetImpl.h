#pragma once

#include "AttributeWidget.h"
#include <QGridLayout>
#include <QLineEdit>

namespace Urho3DEditor
{

/// String Attribute Editor.
class StringAttributeWidget : public AttributeWidget
{
    Q_OBJECT

public:
    /// Construct.
    StringAttributeWidget(QWidget* parent = nullptr);

    /// Mark/unmark value as undefined.
    void SetUndefined(bool undefined);

    /// @see AttributeWidget::GetVariantType
    virtual Urho3D::VariantType GetVariantType() const override { return Urho3D::VAR_STRING; }
    /// @see AttributeWidget::GetValue
    virtual void GetValue(Urho3D::Variant& result) const override;
    /// @see AttributeWidget::SetValue
    virtual void SetValue(const Urho3D::Variant& value) override;
    /// @see AttributeWidget::SetMergedValue
    virtual void SetMergedValue(const QVector<Urho3D::Variant>& value) override;

private slots:
    /// Handle text edited.
    void HandleTextEdited();

private:
    /// Layout.
    QGridLayout* layout_;
    /// Widget.
    QLineEdit* widget_;
    /// Is undefined?
    bool undefined_;
    /// Value.
    Urho3D::String value_;
};

}
