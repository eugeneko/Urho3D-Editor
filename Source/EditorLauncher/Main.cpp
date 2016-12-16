#include "../Editor/EditorApplication.h"

int main( int argc, char **argv )
{
    Urho3D::EditorApplication a(argc, argv, new Urho3D::Context());
    return a.Run();
}
