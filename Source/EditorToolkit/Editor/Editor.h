#pragma once

#include <Urho3D/Math/Ray.h>
#include <Urho3D/Input/Input.h>

namespace Urho3D
{

class UI;
class EditorViewportLayout;

/// Interface of editor input provider.
class AbstractEditorInput : public Object
{
    URHO3D_OBJECT(AbstractEditorInput, Object);

public:
    /// Construct.
    AbstractEditorInput(Context* context) : Object(context) {}
    /// Reset grab.
    virtual void ResetGrab() = 0;

    /// Set mouse mode.
    virtual void SetMouseMode(MouseMode mouseMode) = 0;
    /// Return whether the UI is focused.
    virtual bool IsUIFocused() const = 0;
    /// Return whether the UI is hovered.
    virtual bool IsUIHovered() const = 0;

    /// Return whether the key is down.
    virtual bool IsKeyDown(int key) const = 0;
    /// Return whether the key is pressed.
    virtual bool IsKeyPressed(int key) const = 0;
    /// Return whether the mouse button is down.
    virtual bool IsMouseButtonDown(int mouseButton) const = 0;
    /// Return whether the mouse button is pressed.
    virtual bool IsMouseButtonPressed(int mouseButton) const = 0;
    /// Return mouse position.
    virtual IntVector2 GetMousePosition() const = 0;
    /// Return mouse move.
    virtual IntVector2 GetMouseMove() const = 0;
    /// Return mouse wheel delta.
    virtual int GetMouseWheelMove() const = 0;
    /// Return mouse ray in 3D.
    virtual Ray GetMouseRay() const = 0;

    /// Grab key.
    virtual void GrabKey(int key) = 0;
    /// Checks whether key is grabbed.
    virtual bool IsKeyGrabbed(int key) const = 0;
    /// Grab mouse button input.
    virtual void GrabMouseButton(int mouseButton) = 0;
    /// Checks whether mouse button is grabbed.
    virtual bool IsMouseButtonGrabbed(int mouseButton) const = 0;
    /// Grab mouse button input.
    virtual void GrabMouseMove() = 0;
    /// Checks whether mouse button is grabbed.
    virtual bool IsMouseMoveGrabbed() const = 0;
};

/// Interface of editor overlay.
class AbstractEditorOverlay : public Object
{
    URHO3D_OBJECT(AbstractEditorOverlay, Object);

public:
    /// Construct.
    AbstractEditorOverlay(Context* context) : Object(context) {}
    /// General update.
    virtual void Update(AbstractEditorInput& input, float timeStep);
    /// Post-render update. It is safe to render here.
    virtual void PostRenderUpdate(AbstractEditorInput& input);
};

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
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

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

    /// Reset grab.
    void ResetGrab() override;

    /// Set mouse mode.
    void SetMouseMode(Urho3D::MouseMode mouseMode) override;
    /// Return whether the UI is focused.
    bool IsUIFocused() const override;
    /// Return whether the UI is hovered.
    bool IsUIHovered() const override;

    /// Return whether the key is down.
    bool IsKeyDown(int key) const override;
    /// Return whether the key is pressed.
    bool IsKeyPressed(int key) const override;
    /// Return whether the mouse button is down.
    bool IsMouseButtonDown(int mouseButton) const override;
    /// Return whether the mouse button is pressed.
    bool IsMouseButtonPressed(int mouseButton) const override;
    /// Return mouse position.
    IntVector2 GetMousePosition() const override;
    /// Return mouse move.
    IntVector2 GetMouseMove() const override;
    /// Return mouse wheel delta.
    int GetMouseWheelMove() const override;
    /// Return mouse ray in 3D.
    Ray GetMouseRay() const override;

    /// Grab key.
    void GrabKey(int key) override;
    /// Checks whether key is grabbed.
    bool IsKeyGrabbed(int key) const override;
    /// Grab mouse button input.
    void GrabMouseButton(int mouseButton) override;
    /// Checks whether mouse button is grabbed.
    bool IsMouseButtonGrabbed(int mouseButton) const override;
    /// Grab mouse button input.
    void GrabMouseMove() override;
    /// Checks whether mouse button is grabbed.
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
