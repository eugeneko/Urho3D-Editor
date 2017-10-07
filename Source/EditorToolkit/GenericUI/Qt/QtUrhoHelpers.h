#pragma once

#include <QString>
#include <QVector>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Container/Ptr.h>

namespace Urho3D
{

class Serializable;

}

namespace Urho3D
{

/// Convert from Qt to Urho3D string.
inline String Cast(const QString& value) { return value.toStdString().c_str(); }

/// Convert from Urho3D to Qt string.
inline QString Cast(const String& value) { return value.CString(); }

/// Convert mouse button code from Urho3D to Qt.
inline Qt::MouseButton ConvertMouseButton(int button)
{
    switch (button)
    {
    case MOUSEB_LEFT: return Qt::LeftButton;
    case MOUSEB_RIGHT: return Qt::RightButton;
    case MOUSEB_MIDDLE: return Qt::MiddleButton;
    case MOUSEB_X1: return Qt::XButton1;
    case MOUSEB_X2: return Qt::XButton2;
    default: return Qt::NoButton;
    }
}

/// Vector of serializables.
using SerializableVector = QVector<Serializable*>;

/// Gather serializables from container.
template <class T>
SerializableVector GatherSerializables(const T& container)
{
    SerializableVector result;
    for (Serializable* item : container)
        result.push_back(item);
    return result;
}

}
