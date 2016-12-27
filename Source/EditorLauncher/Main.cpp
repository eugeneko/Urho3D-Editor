#include "../Editor/EditorApplication.h"

#include <Urho3D/Core/Main.h>

int Main()
{
    int argc = 0;
    char** argv = 0;
    Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context());
    Urho3D::EditorApplication editor(argc, argv, context);
    editor.AddPlugin(Urho3D::MakeShared<Urho3D::SceneEditorPlugin>());

    return editor.Run();
}

URHO3D_DEFINE_MAIN(Main());
