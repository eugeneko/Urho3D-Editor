#include "Editor.h"
#include "EditorViewportLayout.h"
#include "../AbstractUI/AbstractUI.h"
#include <Urho3D/Core/CoreEvents.h>

namespace Urho3D
{

//////////////////////////////////////////////////////////////////////////
StandardEditorContext::StandardEditorContext(Context* context, EditorViewportLayout* viewportLayout)
    : AbstractEditorContext(context)
    , viewportLayout_(viewportLayout)
{
}

Ray StandardEditorContext::GetMouseRay() const
{
    return viewportLayout_ ? viewportLayout_->GetCurrentCameraRay() : Ray();
}

Camera* StandardEditorContext::GetCurrentCamera() const
{
    return &viewportLayout_->GetCurrentCamera();
}

//////////////////////////////////////////////////////////////////////////
Editor::Editor(AbstractMainWindow& mainWindow)
    : Object(mainWindow.GetContext())
    , input_(mainWindow.GetInput())
{
    SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(Editor, HandleUpdate));
    SubscribeToEvent(Urho3D::E_POSTRENDERUPDATE, URHO3D_HANDLER(Editor, HandlePostRenderUpdate));
}

void Editor::AddOverlay(AbstractEditorOverlay* overlay)
{
    overlays_.Push(SharedPtr<AbstractEditorOverlay>(overlay));
}

void Editor::AddSubsystem(Object* subsystem)
{
    subsystems_.Push(SharedPtr<Object>(subsystem));
}

void Editor::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    const float timeStep = eventData[Update::P_TIMESTEP].GetFloat();
    input_->ResetGrab();
    for (AbstractEditorOverlay* overlay : overlays_)
        overlay->Update(*input_, *editorContext_, timeStep);
}

void Editor::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    for (AbstractEditorOverlay* overlay : overlays_)
        overlay->PostRenderUpdate(*input_, *editorContext_);
}

}

