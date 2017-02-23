#include "SceneOverlay.h"

namespace Urho3DEditor
{

bool SceneInputInterface::TryConsumeMouseButton(Qt::MouseButton mouseButton)
{
    if (!IsMouseButtonConsumed(mouseButton))
    {
        ConsumeMouseButton(mouseButton);
        return true;
    }
    return false;
}

void SceneOverlay::Update(SceneInputInterface& /*input*/, float /*timeStep*/)
{
}

void SceneOverlay::PostRenderUpdate(SceneInputInterface& /*input*/)
{
}

}

