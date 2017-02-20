#include "AttributeWidgetImpl.h"
#include "../Bridge.h"
#include <QMouseEvent>

namespace Urho3DEditor
{

namespace
{

/// Convert vector-like type into vector.
template <class T> QVector<double> SliceVector(const T& /*value*/) { return{}; }
template <> QVector<double> SliceVector<Urho3D::Vector2>(const Urho3D::Vector2& value) { return{ value.x_, value.y_ }; }
template <> QVector<double> SliceVector<Urho3D::Vector3>(const Urho3D::Vector3& value) { return{ value.x_, value.y_, value.z_ }; }
template <> QVector<double> SliceVector<Urho3D::Vector4>(const Urho3D::Vector4& value) { return{ value.x_, value.y_, value.z_, value.w_ }; }
template <> QVector<double> SliceVector<Urho3D::Rect>(const Urho3D::Rect& value) { return SliceVector(value.ToVector4()); }
template <> QVector<double> SliceVector<Urho3D::Color>(const Urho3D::Color& value) { return{ value.r_, value.g_, value.b_, value.a_ }; }

/// Convert vector-like variants into vector.
QVector<double> SliceVectorVariant(const Urho3D::Variant& variant)
{
    switch (variant.GetType())
    {
    case Urho3D::VAR_VECTOR2: return SliceVector(variant.GetVector2());
    case Urho3D::VAR_VECTOR3: return SliceVector(variant.GetVector3());
    case Urho3D::VAR_VECTOR4: return SliceVector(variant.GetVector4());
    case Urho3D::VAR_RECT:    return SliceVector(variant.GetRect());
    case Urho3D::VAR_COLOR:   return SliceVector(variant.GetColor());
    }
    return{};
}

/// Convert vector of vector-like variants into vector of variant vectors.
QVector<VariantArray> SliceVectorVariantArray(const VariantArray& variantArray)
{
    QVector<VariantArray> result;
    for (int i = 0; i < variantArray.size(); ++i)
    {
        const QVector<double> elements = SliceVectorVariant(variantArray[i]);

        // Initialize
        if (result.empty())
        {
            result.resize(elements.size());
            for (VariantArray& slice : result)
                slice.resize(variantArray.size());
        }

        // Fill
        for (int j = 0; j < qMin(elements.size(), result.size()); ++j)
            result[j][i] = elements[j];
    }
    return result;
}

/// Compute number of components of variant.
int GetVaraintComponentsCount(Urho3D::VariantType type)
{
    switch (type)
    {
    case Urho3D::VAR_VECTOR2:
        return 2;
    case Urho3D::VAR_VECTOR3:
        return 3;
    case Urho3D::VAR_VECTOR4:
    case Urho3D::VAR_RECT:
    case Urho3D::VAR_COLOR:
        return 4;
    case Urho3D::VAR_QUATERNION:
    default:
        return 1;
    }
}

}

//////////////////////////////////////////////////////////////////////////
StringAttributeWidget::StringAttributeWidget(QWidget* parent /*= nullptr*/)
    : SolidAttributeWidget(parent)
    , layout_(new QGridLayout(this))
    , widget_(new QLineEdit(this))
{
    connect(widget_, &QLineEdit::textEdited, this, &StringAttributeWidget::HandleTextEdited);
    connect(widget_, &QLineEdit::returnPressed, this, &StringAttributeWidget::valueCommitted);

    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->addWidget(widget_, 0, 0);
    setLayout(layout_);

    SetUndefined(false);
}

void StringAttributeWidget::SetUndefined(bool undefined)
{
    SolidAttributeWidget::SetUndefined(undefined);
    widget_->setPlaceholderText(undefined ? "(undefined)" : "(empty)");
}

void StringAttributeWidget::GetValue(Urho3D::Variant& result) const
{
    if (!IsUndefined())
        result = value_;
}

bool StringAttributeWidget::SetValue(const Urho3D::Variant& value)
{
    value_ = value.ToString();
    widget_->setText(Cast(value_));
    return true;
}

void StringAttributeWidget::HandleTextEdited()
{
    SetUndefined(false);
    value_ = Cast(widget_->text());
    emit valueChanged();
}

//////////////////////////////////////////////////////////////////////////
DoubleAttributeWidget::DoubleAttributeWidget(Urho3D::VariantType type, QWidget* parent /*= nullptr*/)
    : SolidAttributeWidget(parent)
    , type_(type)
    , layout_(new QGridLayout(this))
    , widget_(new QLineEdit(this))
    , labelWidget_(new QLabel(QChar(0x00B1), widget_))
{
    labelWidget_->setAttribute(Qt::WA_TranslucentBackground);
    labelWidget_->installEventFilter(this);
    labelWidget_->setCursor(Qt::SizeHorCursor);
    widget_->setValidator(new QDoubleValidator(-Urho3D::M_INFINITY, Urho3D::M_INFINITY, 2, this));

    connect(widget_, &QLineEdit::textEdited, this, &DoubleAttributeWidget::HandleTextEdited);
    connect(widget_, &QLineEdit::returnPressed, this, &DoubleAttributeWidget::valueCommitted);

    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->addWidget(widget_, 0, 0);
    setLayout(layout_);

    SetUndefined(false);
}

void DoubleAttributeWidget::SetUndefined(bool undefined)
{
    SolidAttributeWidget::SetUndefined(undefined);
    widget_->setPlaceholderText(undefined ? "(?)" : "0");
}

void DoubleAttributeWidget::GetValue(Urho3D::Variant& result) const
{
    if (IsUndefined())
        return;

    switch (type_)
    {
    case Urho3D::VAR_FLOAT:
        result = static_cast<float>(value_);
        break;
    case Urho3D::VAR_DOUBLE:
        result = static_cast<double>(value_);
        break;
    default:
        break;
    }
}

bool DoubleAttributeWidget::SetValue(const Urho3D::Variant& value)
{
    value_ = value.GetDouble();
    widget_->setText(value.IsEmpty() ? "" : QString::number(value_));
    return true;
}

void DoubleAttributeWidget::HandleTextEdited()
{
    SetUndefined(false);
    value_ = widget_->text().toDouble();
    emit valueChanged();
}

bool DoubleAttributeWidget::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::MouseMove:
        return HandleMouseMoveEvent(static_cast<QMouseEvent*>(event));
    case QEvent::MouseButtonPress:
        return HandleMouseButtonPressEvent(static_cast<QMouseEvent*>(event));
    case QEvent::MouseButtonRelease:
        return HandleMouseButtonReleaseEvent(static_cast<QMouseEvent*>(event));
    }
    return QObject::eventFilter(watched, event);
}

bool DoubleAttributeWidget::HandleMouseButtonPressEvent(QMouseEvent* event)
{
    prevPosition_ = event->globalPos();
    isMouseDragging_ = true;
    event->accept();
    return true;
}

bool DoubleAttributeWidget::HandleMouseButtonReleaseEvent(QMouseEvent* event)
{
    if (isMouseDragging_)
        emit valueCommitted();
    isMouseDragging_ = false;
    event->accept();
    return true;
}

bool DoubleAttributeWidget::HandleMouseMoveEvent(QMouseEvent* event)
{
    if (!event->buttons().testFlag(Qt::LeftButton) || !isMouseDragging_)
        return false;

    // Compute delta
    const QPoint delta = event->globalPos() - prevPosition_;
    prevPosition_ = event->globalPos();

    // Wrap mouse
    const QPoint windowSize(window()->width() - 2, window()->height() - 2);
    const QPoint windowPosition = window()->mapToGlobal(QPoint(1, 1));
    const QRect windowRect(windowPosition, windowPosition + windowSize);
    if (prevPosition_.x() < windowRect.left())
    {
        prevPosition_.setX(windowRect.right());
        QCursor::setPos(prevPosition_);
    }
    else if (prevPosition_.x() > windowRect.right())
    {
        prevPosition_.setX(windowRect.left());
        QCursor::setPos(prevPosition_);
    }

    // Apply changes
    static const double step = 0.01;
    const int dX = delta.x() % windowSize.x();
    value_ = Urho3D::Round((value_ + dX * step) / step) * step;
    SetValue(value_);
    emit valueChanged();

    event->accept();
    return true;
}

void DoubleAttributeWidget::resizeEvent(QResizeEvent* event)
{
    const int labelWidth = labelWidget_->sizeHint().width() * 3 / 2;
    labelWidget_->resize(labelWidth, widget_->height());
    labelWidget_->move(widget_->width() - labelWidth, 0);
}

//////////////////////////////////////////////////////////////////////////
FloatVectorAttributeWidget::FloatVectorAttributeWidget(Urho3D::VariantType type, QWidget* parent /*= nullptr*/)
    : AttributeWidget(parent)
    , type_(type)
    , numComponents_(GetVaraintComponentsCount(type_))
    , layout_(new QGridLayout(this))
{
    layout_->setContentsMargins(0, 0, 0, 0);
    for (int i = 0; i < numComponents_; ++i)
    {
        DoubleAttributeWidget* widget = new DoubleAttributeWidget(Urho3D::VAR_FLOAT, this);
        connect(widget, &DoubleAttributeWidget::valueChanged, this, &FloatVectorAttributeWidget::valueChanged);
        connect(widget, &DoubleAttributeWidget::valueCommitted, this, &FloatVectorAttributeWidget::valueCommitted);
        layout_->addWidget(widget, 0, i);
        fieldWidgets_.push_back(widget);
    }

    setLayout(layout_);
}

void FloatVectorAttributeWidget::GetValue(Urho3D::Variant& result) const
{
    using namespace Urho3D;
    switch (type_)
    {
    case VAR_VECTOR2:
        {
            Vector2 value = result.GetVector2();
            SetIfDefined(0, value.x_);
            SetIfDefined(1, value.y_);
            result = value;
            break;
        }
    case VAR_VECTOR3:
        {
            Vector3 value = result.GetVector3();
            SetIfDefined(0, value.x_);
            SetIfDefined(1, value.y_);
            SetIfDefined(2, value.z_);
            result = value;
            break;
        }
    case VAR_VECTOR4:
        {
            Vector4 value = result.GetVector4();
            SetIfDefined(0, value.x_);
            SetIfDefined(1, value.y_);
            SetIfDefined(2, value.z_);
            SetIfDefined(3, value.w_);
            result = value;
            break;
        }
    case VAR_RECT:
        {
            Vector4 value = result.GetRect().ToVector4();
            SetIfDefined(0, value.x_);
            SetIfDefined(1, value.y_);
            SetIfDefined(2, value.z_);
            SetIfDefined(3, value.w_);
            result = Rect(value);
            break;
        }
    case VAR_COLOR:
        {
            Color value = result.GetColor();
            SetIfDefined(0, value.r_);
            SetIfDefined(1, value.g_);
            SetIfDefined(2, value.b_);
            SetIfDefined(3, value.a_);
            result = value;
            break;
        }
    default:
        break;
    }
}

bool FloatVectorAttributeWidget::SetValue(const Urho3D::Variant& value)
{
    using namespace Urho3D;
    if (value.IsEmpty())
    {
        for (DoubleAttributeWidget* field : fieldWidgets_)
            field->SetValue(Variant::EMPTY);
    }
    else
    {
        const QVector<double> elements = SliceVectorVariant(value);
        if (elements.size() != numComponents_)
            return false;

        for (int i = 0; i < elements.size(); ++i)
            fieldWidgets_[i]->SetValue(elements[i]);
    }
    return true;
}

void FloatVectorAttributeWidget::SetMergedValue(const VariantArray& values)
{
    if (values.empty())
    {
        SetValue(Urho3D::Variant::EMPTY);
    }
    else
    {
        const QVector<VariantArray> elements = SliceVectorVariantArray(values);
        for (int i = 0; i < qMin(elements.size(), fieldWidgets_.size()); ++i)
            fieldWidgets_[i]->SetMergedValue(elements[i]);
        for (int i = elements.size(); i < fieldWidgets_.size(); ++i)
        {
            fieldWidgets_[i]->SetValue(Urho3D::Variant::EMPTY);
            fieldWidgets_[i]->SetUndefined(true);
        }
    }
}

void FloatVectorAttributeWidget::SetIfDefined(int index, float& value) const
{
    if (fieldWidgets_.size() > index && !fieldWidgets_[index]->IsUndefined())
        value = static_cast<float>(fieldWidgets_[index]->GetRawValue());

}

//////////////////////////////////////////////////////////////////////////
QuaternionAttributeWidget::QuaternionAttributeWidget(QWidget* parent /*= nullptr*/)
    : FloatVectorAttributeWidget(Urho3D::VAR_VECTOR3, parent)
{
}

void QuaternionAttributeWidget::GetValue(Urho3D::Variant& result) const
{
    // Get Euler angles
    Urho3D::Variant eulerAngles = result.GetQuaternion().EulerAngles();
    FloatVectorAttributeWidget::GetValue(eulerAngles);

    const Urho3D::Vector3 xyz = eulerAngles.GetVector3();
    result = Urho3D::Quaternion(xyz.x_, xyz.y_, xyz.z_);
}

bool QuaternionAttributeWidget::SetValue(const Urho3D::Variant& value)
{
    return FloatVectorAttributeWidget::SetValue(value.GetQuaternion().EulerAngles());
}

void QuaternionAttributeWidget::SetMergedValue(const VariantArray& values)
{
    VariantArray anglesArray(values.size());
    for (int i = 0; i < values.size(); ++i)
        anglesArray[i] = values[i].GetQuaternion().EulerAngles();
    FloatVectorAttributeWidget::SetMergedValue(anglesArray);
}

}
