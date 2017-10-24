#pragma once

namespace Urho3D
{

/// Editor current viewport changed.
URHO3D_EVENT(E_EDITORCURRENTVIEWPORTCHANGED, EditorCurrentViewportChanged)
{
    URHO3D_PARAM(P_VIEWPORTLAYOUT, ViewportLayout); // EditorViewportLayout ptr
    URHO3D_PARAM(P_CAMERA, Camera);                 // Camera ptr
}

/// Editor selection changed.
// URHO3D_EVENT(E_EDITORSELECTIONCHANGED, EditorSelectionChanged)
// {
//     URHO3D_PARAM(P_SELECTION, Selection);   // Selection ptr
// }

}
