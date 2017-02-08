#include "VariantEditorWidgetImpl.h"
#include "../Bridge.h"

#include <QLabel>
namespace Urho3DEditor
{

StringEditorWidget::StringEditorWidget(QWidget* parent /*= nullptr*/)
    : VariantEditorWidget(parent)
    , layout_(new QGridLayout(this))
    , widget_(new QLineEdit(this))
    , undefined_(false)
{
    layout_->addWidget(widget_, 0, 0);
    setLayout(layout_);

    SetUndefined(false);
}

void StringEditorWidget::SetUndefined(bool undefined)
{
    undefined_ = undefined;
    widget_->setPlaceholderText(undefined_ ? "(undefined)" : "(empty)");
}

void StringEditorWidget::GetValue(Urho3D::Variant& result) const
{
    if (!undefined_)
        result = value_;
}

void StringEditorWidget::SetValue(const Urho3D::Variant& value)
{
    value_ = value.ToString();
    widget_->setText(Cast(value_));
}

void StringEditorWidget::HandleTextEdited()
{
    SetUndefined(false);
    value_ = Cast(widget_->text());
    emit valueChanged();
}

}
