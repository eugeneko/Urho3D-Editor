#pragma once

#include <QString>
#include <Urho3D/Container/Str.h>

namespace Urho3DEditor
{

/// Convert from Qt to Urho3D string.
inline Urho3D::String Cast(const QString& value) { return value.toStdString().c_str(); }

/// Convert from Urho3D to Qt string.
inline QString Cast(const Urho3D::String& value) { return value.CString(); }

}
