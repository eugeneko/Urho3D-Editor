#include <Urho3DEditor/Application.h>

#include <Urho3D/Core/Main.h>

int QtEditorMain()
{
    Urho3DEditor::Application editor(0, nullptr);
    return editor.Run();
}

URHO3D_DEFINE_MAIN(QtEditorMain());
