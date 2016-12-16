#include "EditorApplication.h"

namespace Urho3D
{

EditorApplication::EditorApplication(int argc, char** argv, Urho3D::Context* context)
    : QApplication(argc, argv)
    , context_(context ? context : new Context())
{
}

int EditorApplication::Run()
{
    return exec();
}

}
