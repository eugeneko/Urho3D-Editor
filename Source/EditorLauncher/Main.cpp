#include "../Editor/Application.h"

#include <Urho3D/Core/Main.h>

int Main()
{
    int argc = 0;
    char** argv = 0;
    Urho3DEditor::Application editor(argc, argv);
//     editor.AddPlugin(Urho3D::MakeShared<Urho3DEditor::SceneEditorPlugin>());

    return editor.Run();
}

URHO3D_DEFINE_MAIN(Main());
