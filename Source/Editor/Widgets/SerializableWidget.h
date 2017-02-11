#pragma once

#include "../Bridge.h"
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

private:
    /// Layout.
    QGridLayout* layout_;
    /// Serializables.
    SerializableVector serializables_;
    /// Attribute editors.
    QVector<AttributeWidget*> attributes_;
};

}
