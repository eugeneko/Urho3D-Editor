#pragma once

#include "VariantEditorWidget.h"
#include <QGridLayout>
#include <QLineEdit>

namespace Urho3DEditor
{

/// Variant Editor Widget: String.
class StringEditorWidget : public VariantEditorWidget
{
    Q_OBJECT

public:
    /// Construct.
    StringEditorWidget(QWidget* parent = nullptr);

    /// Mark/unmark value as undefined.
    void SetUndefined(bool undefined);

    /// @see VariantEditorWidget::GetVariantType
    virtual Urho3D::VariantType GetVariantType() const override { return Urho3D::VAR_STRING; }
    /// @see VariantEditorWidget::GetValue
    virtual void GetValue(Urho3D::Variant& result) const override;
    /// @see VariantEditorWidget::SetValue
    virtual void SetValue(const Urho3D::Variant& value) override;

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
