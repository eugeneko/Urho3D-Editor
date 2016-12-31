#include "../Editor/Application.h"

#include <Urho3D/Core/Main.h>

int Main()
{
    int argc = 0;
    char** argv = 0;
    Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context());
    Urho3DEditor::Application editor(argc, argv, context);
//     editor.AddPlugin(Urho3D::MakeShared<Urho3DEditor::SceneEditorPlugin>());

    return editor.Run();
}

URHO3D_DEFINE_MAIN(Main());
