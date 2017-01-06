#pragma once

#include <QString>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Input/InputEvents.h>

namespace Urho3DEditor
{

/// Convert from Qt to Urho3D string.
inline Urho3D::String Cast(const QString& value) { return value.toStdString().c_str(); }

/// Convert from Urho3D to Qt string.
inline QString Cast(const Urho3D::String& value) { return value.CString(); }

/// Convert mouse button code from Urho3D to Qt.
inline Qt::MouseButton ConvertMouseButton(int button)
{
    switch (button)
    {
    case Urho3D::MOUSEB_LEFT: return Qt::LeftButton;
    case Urho3D::MOUSEB_RIGHT: return Qt::RightButton;
    case Urho3D::MOUSEB_MIDDLE: return Qt::MiddleButton;
    case Urho3D::MOUSEB_X1: return Qt::XButton1;
    case Urho3D::MOUSEB_X2: return Qt::XButton2;
    default: return Qt::NoButton;
    }
}

}
