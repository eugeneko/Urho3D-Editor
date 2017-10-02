#include "Editor.h"
#include "EditorViewportLayout.h"
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/UI/UI.h>

namespace Urho3D
{

//////////////////////////////////////////////////////////////////////////
UrhoEditorInput::UrhoEditorInput(Context* context)
    : AbstractEditorInput(context)
    , input_(GetSubsystem<Input>())
    , ui_(GetSubsystem<UI>())
{
}

void UrhoEditorInput::SetViewportLayout(EditorViewportLayout* viewportLayout)
{
    viewportLayout_ = viewportLayout;
}

void UrhoEditorInput::ResetGrab()
{
    grabbedKeys_.Clear();
    /// Grabbed mouse buttons.
    grabbedMouseButtons_.Clear();
    /// Whether the mouse movement is grabbed.
    grabbedMouseMove_ = false;
}

void UrhoEditorInput::SetMouseMode(Urho3D::MouseMode mouseMode)
{
    input_->SetMouseMode(mouseMode);
}

bool UrhoEditorInput::IsUIFocused() const
{
    return ui_->HasModalElement() || ui_->GetFocusElement();
}

bool UrhoEditorInput::IsUIHovered() const
{
    return !!ui_->GetElementAt(GetMousePosition());
}

bool UrhoEditorInput::IsKeyDown(int key) const
{
    return input_->GetKeyDown(key);
}

bool UrhoEditorInput::IsKeyPressed(int key) const
{
    return input_->GetKeyPress(key);
}

bool UrhoEditorInput::IsMouseButtonDown(int mouseButton) const
{
    return input_->GetMouseButtonDown(mouseButton);
}

bool UrhoEditorInput::IsMouseButtonPressed(int mouseButton) const
{
    return input_->GetMouseButtonPress(mouseButton);
}

IntVector2 UrhoEditorInput::GetMousePosition() const
{
    return input_->GetMousePosition();
}

IntVector2 UrhoEditorInput::GetMouseMove() const
{
    return input_->GetMouseMove();
}

int UrhoEditorInput::GetMouseWheelMove() const
{
    return input_->GetMouseMoveWheel();
}

Ray UrhoEditorInput::GetMouseRay() const
{
    return viewportLayout_ ? viewportLayout_->GetCurrentCameraRay() : Ray();
}

float UrhoEditorInput::GetFarClip() const
{
    return viewportLayout_->GetCurrentCamera().GetFarClip();
}

void UrhoEditorInput::GrabKey(int key)
{
    grabbedKeys_.Insert(key);
}

bool UrhoEditorInput::IsKeyGrabbed(int key) const
{
    return grabbedKeys_.Contains(key);
}

void UrhoEditorInput::GrabMouseButton(int mouseButton)
{
    grabbedMouseButtons_.Insert(mouseButton);
}

bool UrhoEditorInput::IsMouseButtonGrabbed(int mouseButton) const
{
    return grabbedMouseButtons_.Contains(mouseButton);
}

void UrhoEditorInput::GrabMouseMove()
{
    grabbedMouseMove_ = true;
}

bool UrhoEditorInput::IsMouseMoveGrabbed() const
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

