#include "AbstractInput.h"

namespace Urho3D
{

void StandardInput::ResetGrab()
{
    grabbedKeys_.Clear();
    grabbedMouseButtons_.Clear();
    grabbedMouseMove_ = false;
}

void StandardInput::GrabKey(int key)
{
    grabbedKeys_.Insert(key);
}

bool StandardInput::IsKeyGrabbed(int key) const
{
    return grabbedKeys_.Contains(key);
}

void StandardInput::GrabMouseButton(int mouseButton)
{
    grabbedMouseButtons_.Insert(mouseButton);
}

bool StandardInput::IsMouseButtonGrabbed(int mouseButton) const
{
    return grabbedMouseButtons_.Contains(mouseButton);
}

void StandardInput::GrabMouseMove()
{
    grabbedMouseMove_ = true;
}

bool StandardInput::IsMouseMoveGrabbed() const
{
    return grabbedMouseMove_;
}

}
