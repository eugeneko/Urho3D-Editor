#pragma once

#include "../Bridge.h"
#include <Urho3D/Core/Attribute.h>
#include <QWidget>

class QGridLayout;

namespace Urho3DEditor
{

class AttributeWidget;

/// Urho3D::Serializable editor widget.
class SerializableWidget : public QWidget
{
    Q_OBJECT

public:
    /// Construct.
    SerializableWidget(const SerializableVector& serializables, QWidget* parent = nullptr);
    /// Update values of widgets.
    void Update();

signals:
    /// Signals that attribute value has been changed.
    void attributeChanged(const SerializableVector& serializables,
        unsigned attributeIndex, const QVector<Urho3D::Variant>& newValues);
    /// Signals that attribute value has been committed.
    void attributeCommitted(const SerializableVector& serializables, unsigned attributeIndex);

private slots:
    /// Handle attribute changed.
    void HandleAttributeChanged();
    /// Handle attribute committed.
    void HandleAttributeCommitted();

private:
    /// Empty attribute array.
    static const Urho3D::Vector<Urho3D::AttributeInfo> emptyAttribArray;
    /// Gather attribute of multiple serializables to array.
    QVector<Urho3D::Variant> GatherAttribute(unsigned attribIndex) const;

private:
    /// Layout.
    QGridLayout* layout_;

    /// Serializables.
    SerializableVector serializables_;
    /// Real serializable class name.
    Urho3D::String serializableType_;
    /// Attributes.
    const Urho3D::Vector<Urho3D::AttributeInfo>* attributes_;
    /// Attribute editors.
    QVector<AttributeWidget*> attributeWidgets_;
};

}
