#pragma once

#include "EditorInterfaces.h"
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Input/Input.h>

namespace Urho3D
{

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

private:
    /// Handle update.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle post-render update.
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);

private:
    SharedPtr<AbstractEditorInput> input_ = nullptr;
    Vector<SharedPtr<AbstractEditorOverlay>> overlays_;
};

/// Urho Editor input interface.
class UrhoEditorInput : public AbstractEditorInput
{
    URHO3D_OBJECT(UrhoEditorInput, AbstractEditorInput);

public:
    /// Construct.
    UrhoEditorInput(Context* context);
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
    /// \see AbstractEditorInput::GetFarClip
    float GetFarClip() const override;

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
    Input* input_ = nullptr;
    /// UI.
    UI* ui_ = nullptr;
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
