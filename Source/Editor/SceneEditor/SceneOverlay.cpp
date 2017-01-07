#include "SceneOverlay.h"

namespace Urho3DEditor
{

bool SceneOverlay::MouseButtonEvent(const Urho3D::Ray& /*cameraRay*/, Qt::MouseButton /*button*/, bool /*pressed*/, bool /*consumed*/)
{
    return false;
}

void SceneOverlay::Update(const Urho3D::Ray& /*cameraRay*/, float /*timeStep*/)
{
}

void SceneOverlay::PostRenderUpdate(const Urho3D::Ray& /*cameraRay*/)
{
}

}

