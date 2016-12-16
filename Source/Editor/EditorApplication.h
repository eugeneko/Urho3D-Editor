#pragma once

#include <Urho3D/Core/Context.h>
#include <QApplication>

namespace Urho3D
{

class EditorApplication : private QApplication
{
public:
    /// Ctor.
    EditorApplication(int argc, char** argv, Context* context);
    /// Run!
    int Run();

private:
    /// Urho3D context.
    SharedPtr<Context> context_;

};

}

