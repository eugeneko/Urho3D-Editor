#pragma once

#include "EditorInterfaces.h"
#include <Urho3D/Math/Ray.h>

namespace Urho3D
{

class AbstractInput;
class EditorViewportLayout;

/// Editor base.
class Editor : public Object
{
    URHO3D_OBJECT(Editor, Object);

public:
    /// Construct.
    Editor(Context* context);
    /// Set input interface.
    void SetInput(AbstractEditorInput* input) { input_ = input; }
    /// Add overlay.
    /// \todo add priority
    void AddOverlay(AbstractEditorOverlay* overlay);
    /// Add editor subsystem.
    void AddSubsystem(Object* subsystem);

private:
    /// Handle update.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle post-render update.
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);

private:
    SharedPtr<AbstractEditorInput> input_ = nullptr;
    Vector<SharedPtr<AbstractEditorOverlay>> overlays_;
    Vector<SharedPtr<Object>> subsystems_;
};

/// Standard Editor input.
class StandardEditorInput : public AbstractEditorInput
{
    URHO3D_OBJECT(StandardEditorInput, AbstractEditorInput);

public:
    /// Construct.
    StandardEditorInput(Context* context, AbstractInput* input, EditorViewportLayout* viewportLayout);
    /// Set viewport layout.
    /// \todo replace with interface
    void SetViewportLayout(EditorViewportLayout* viewportLayout);

    /// \see AbstractEditorInput::GetFarClip
    void ResetGrab() override;

    /// \see AbstractEditorInput::SetMouseMode
    void SetMouseMode(MouseMode mouseMode) override;
    /// \see AbstractEditorInput::IsUIFocused
    bool IsUIFocused() const override;
    /// \see AbstractEditorInput::IsUIHovered
    bool IsUIHovered() const override;

    /// \see AbstractEditorInput::IsKeyDown
    bool IsKeyDown(int key) const override;
    /// \see AbstractEditorInput::IsKeyPressed
    bool IsKeyPressed(int key) const override;
    /// \see AbstractEditorInput::IsMouseButtonDown
    bool IsMouseButtonDown(int mouseButton) const override;
    /// \see AbstractEditorInput::IsMouseButtonPressed
    bool IsMouseButtonPressed(int mouseButton) const override;
    /// \see AbstractEditorInput::GetMousePosition
    IntVector2 GetMousePosition() const override;
    /// \see AbstractEditorInput::GetMouseMove
    IntVector2 GetMouseMove() const override;
    /// \see AbstractEditorInput::GetMouseWheelMove
    int GetMouseWheelMove() const override;
    /// \see AbstractEditorInput::GetMouseRay
    Ray GetMouseRay() const override;
    /// \see AbstractEditorInput::GetCurrentCamera
    Camera* GetCurrentCamera() const override;

    /// \see AbstractEditorInput::GrabKey
    void GrabKey(int key) override;
    /// \see AbstractEditorInput::IsKeyGrabbed
    bool IsKeyGrabbed(int key) const override;
    /// \see AbstractEditorInput::GrabMouseButton
    void GrabMouseButton(int mouseButton) override;
    /// \see AbstractEditorInput::IsMouseButtonGrabbed
    bool IsMouseButtonGrabbed(int mouseButton) const override;
    /// \see AbstractEditorInput::GrabMouseMove
    void GrabMouseMove() override;
    /// \see AbstractEditorInput::IsMouseMoveGrabbed
    bool IsMouseMoveGrabbed() const override;

private:
    /// Input.
    AbstractInput* input_ = nullptr;
    /// Viewport layout.
    EditorViewportLayout* viewportLayout_ = nullptr;

    /// Grabbed keys.
    HashSet<int> grabbedKeys_;
    /// Grabbed mouse buttons.
    HashSet<int> grabbedMouseButtons_;
    /// Whether the mouse movement is grabbed.
    bool grabbedMouseMove_ = false;
};

}
