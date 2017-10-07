#include "Editor.h"
#include "EditorViewportLayout.h"
#include "../GenericUI/GenericUI.h"
#include <Urho3D/Core/CoreEvents.h>

namespace Urho3D
{

//////////////////////////////////////////////////////////////////////////
StandardEditorInput::StandardEditorInput(Context* context, AbstractInput* input, EditorViewportLayout* viewportLayout)
    : AbstractEditorInput(context)
    , input_(input)
    , viewportLayout_(viewportLayout)
{
}

void StandardEditorInput::ResetGrab()
{
    grabbedKeys_.Clear();
    /// Grabbed mouse buttons.
    grabbedMouseButtons_.Clear();
    /// Whether the mouse movement is grabbed.
    grabbedMouseMove_ = false;
}

void StandardEditorInput::SetMouseMode(Urho3D::MouseMode mouseMode)
{
    input_->SetMouseMode(mouseMode);
}

bool StandardEditorInput::IsUIFocused() const
{
    return input_->IsUIFocused();
}

bool StandardEditorInput::IsUIHovered() const
{
    return input_->IsUIHovered();
}

bool StandardEditorInput::IsKeyDown(int key) const
{
    return input_->IsKeyDown(key);
}

bool StandardEditorInput::IsKeyPressed(int key) const
{
    return input_->IsKeyPressed(key);
}

bool StandardEditorInput::IsMouseButtonDown(int mouseButton) const
{
    return input_->IsMouseButtonDown(mouseButton);
}

bool StandardEditorInput::IsMouseButtonPressed(int mouseButton) const
{
    return input_->IsMouseButtonPressed(mouseButton);
}

IntVector2 StandardEditorInput::GetMousePosition() const
{
    return input_->GetMousePosition();
}

IntVector2 StandardEditorInput::GetMouseMove() const
{
    return input_->GetMouseMove();
}

int StandardEditorInput::GetMouseWheelMove() const
{
    return input_->GetMouseWheelMove();
}

Ray StandardEditorInput::GetMouseRay() const
{
    return viewportLayout_ ? viewportLayout_->GetCurrentCameraRay() : Ray();
}

Camera* StandardEditorInput::GetCurrentCamera() const
{
    return &viewportLayout_->GetCurrentCamera();
}

void StandardEditorInput::GrabKey(int key)
{
    grabbedKeys_.Insert(key);
}

bool StandardEditorInput::IsKeyGrabbed(int key) const
{
    return grabbedKeys_.Contains(key);
}

void StandardEditorInput::GrabMouseButton(int mouseButton)
{
    grabbedMouseButtons_.Insert(mouseButton);
}

bool StandardEditorInput::IsMouseButtonGrabbed(int mouseButton) const
{
    return grabbedMouseButtons_.Contains(mouseButton);
}

void StandardEditorInput::GrabMouseMove()
{
    grabbedMouseMove_ = true;
}

bool StandardEditorInput::IsMouseMoveGrabbed() const
{
    return grabbedMouseMove_;
}

//////////////////////////////////////////////////////////////////////////
Editor::Editor(Context* context)
    : Object(context)
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
        overlay->Update(*input_, timeStep);
}

void Editor::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    for (AbstractEditorOverlay* overlay : overlays_)
        overlay->PostRenderUpdate(*input_);
}

}

