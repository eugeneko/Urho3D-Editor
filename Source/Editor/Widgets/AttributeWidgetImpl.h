#pragma once

#include "AttributeWidget.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

namespace Urho3DEditor
{

/// String Attribute Editor.
class StringAttributeWidget : public SolidAttributeWidget
{
    Q_OBJECT

public:
    /// Construct.
    StringAttributeWidget(QWidget* parent = nullptr);

    /// Set whether the value is undefined.
    virtual void SetUndefined(bool undefined) override;

    /// @see AttributeWidget::GetValue
    virtual void GetValue(Urho3D::Variant& result) const override;
    /// @see AttributeWidget::SetValue
    virtual bool SetValue(const Urho3D::Variant& value) override;

private slots:
    /// Handle text edited.
    void HandleTextEdited();

private:
    /// Layout.
    QGridLayout* layout_;
    /// Widget.
    QLineEdit* widget_;
    /// Value.
    Urho3D::String value_;
};

/// Double Attribute Editor.
class DoubleAttributeWidget : public SolidAttributeWidget
{
    Q_OBJECT

public:
    /// Construct.
    DoubleAttributeWidget(Urho3D::VariantType type, QWidget* parent = nullptr);

    /// Get value as is.
    double GetRawValue() const { return value_; }

    /// @see SolidAttributeWidget::SetUndefined
    virtual void SetUndefined(bool undefined) override;
    /// @see AttributeWidget::GetValue
    virtual void GetValue(Urho3D::Variant& result) const override;
    /// @see AttributeWidget::SetValue
    virtual bool SetValue(const Urho3D::Variant& value) override;

private slots:
    /// Handle text edited.
    void HandleTextEdited();

private:
    /// Mouse button press.
    bool HandleMouseButtonPressEvent(QMouseEvent* event);
    /// Mouse button release.
    bool HandleMouseButtonReleaseEvent(QMouseEvent* event);
    /// Mouse event.
    bool HandleMouseMoveEvent(QMouseEvent* event);
    /// @see QObject::mouseMoveEvent
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
    /// @see QWidget::resizeEvent
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    /// Type.
    const Urho3D::VariantType type_;

    /// Layout.
    QGridLayout* layout_;
    /// Widget.
    QLineEdit* widget_;
    /// Control label.
    QLabel* labelWidget_;
    /// Value.
    double value_ = 0;

    /// Whether the drag is in progress.
    bool isMouseDragging_ = false;
    /// Previous mouse position.
    QPoint prevPosition_;

};

/// Vector Attribute Editor.
class FloatVectorAttributeWidget : public AttributeWidget
{
    Q_OBJECT

public:
    /// Construct.
    FloatVectorAttributeWidget(Urho3D::VariantType type, QWidget* parent = nullptr);

    /// @see AttributeWidget::GetValue
    virtual void GetValue(Urho3D::Variant& result) const override;
    /// @see AttributeWidget::SetValue
    virtual bool SetValue(const Urho3D::Variant& value) override;
    /// @see AttributeWidget::SetMergedValue
    virtual void SetMergedValue(const VariantArray& values) override;

private:
    /// Set value if defined.
    void SetIfDefined(int index, float& value) const;

private:
    /// Type.
    const Urho3D::VariantType type_;
    /// Number of components.
    const int numComponents_;

    /// Layout.
    QGridLayout* layout_;
    /// Field widget.
    QVector<DoubleAttributeWidget*> fieldWidgets_;
};

/// Quaternion Attribute Editor.
class QuaternionAttributeWidget : public FloatVectorAttributeWidget
{
    Q_OBJECT

public:
    /// Construct.
    QuaternionAttributeWidget(QWidget* parent = nullptr);

    /// @see AttributeWidget::GetValue
    virtual void GetValue(Urho3D::Variant& result) const override;
    /// @see AttributeWidget::SetValue
    virtual bool SetValue(const Urho3D::Variant& value) override;
    /// @see AttributeWidget::SetMergedValue
    virtual void SetMergedValue(const VariantArray& values) override;

};

}
