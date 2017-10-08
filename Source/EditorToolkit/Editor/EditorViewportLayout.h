#pragma once

#include "EditorInterfaces.h"
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

class Graphics;

/// Scene viewport.
class EditorViewport : public RefCounted
{
public:
    /// Construct.
    EditorViewport(Context* context, Scene* scene, Camera* camera);
    /// Set transform.
    void SetTransform(const Vector3& position, const Quaternion& rotation);
    /// Set rectangle.
    void SetRect(IntRect rect);

    /// Get camera node.
    Node& GetNode() { return cameraNode_; }
    /// Get viewport camera.
    Camera& GetCamera() const { return *viewportCamera_; }
    /// Get viewport.
    Viewport& GetViewport() const { return *viewport_; }

private:
    /// Viewport camera.
    Camera* sceneCamera_;
    /// Viewport rectangle.
    Rect rect_;

    /// Local camera node.
    Node cameraNode_;
    /// Local camera component.
    Camera& camera_;

    /// Camera.
    Camera* viewportCamera_;
    /// Viewport.
    SharedPtr<Viewport> viewport_;
};

enum class EditorViewportLayoutScheme
{
    /// No viewports.
    Empty,
    /// Single viewport.
    Single,
    /// Vertical split.
    Vertical,
    /// Horizontal split.
    Horizontal,
    /// Quad split.
    Quad,
    /// T-split, 1 viewport on the top, 2 viewports on the bottom.
    Top1_Bottom2,
    /// T-split, 2 viewports on the top, 1 viewport on the bottom.
    Top2_Bottom1,
    /// T-split, 1 viewport on the left, 2 viewports on the right.
    Left1_Right2,
    /// T-split, 2 viewports on the left, 1 viewport on the right.
    Left2_Right1
};

class EditorViewportLayout : public AbstractEditorOverlay
{
    URHO3D_OBJECT(EditorViewportLayout, AbstractEditorOverlay);

public:
    /// Construct.
    EditorViewportLayout(Context* context);

    /// \see AbstractEditorOverlay::Update
    void Update(AbstractInput& input, AbstractEditorContext& editorContext, float timeStep) override;

    /// Set scene.
    void SetScene(Scene* scene);
    /// Set camera for each viewport.
    void SetCameraTransform(Node* cameraNode);
    /// Set layout.
    void SetLayout(EditorViewportLayoutScheme layout);

    /// Compute camera ray.
    Ray ComputeCameraRay(const Viewport& viewport, const IntVector2& mousePosition) const;
    /// Get current camera.
    Camera& GetCurrentCamera();
    /// Get current camera ray.
    const Ray& GetCurrentCameraRay() const { return currentCameraRay_; }

private:
    /// Handle window resize.
    void HandleResize(StringHash eventType, VariantMap& eventData);
    /// Update viewports.
    void UpdateViewports();
    /// Update viewports size.
    void UpdateViewportsSize();
    /// Update active viewport.
    void UpdateActiveViewport(const IntVector2& mousePosition);
    /// Update hovered viewport.
    void UpdateHoveredViewport(const IntVector2& mousePosition);
    /// Get viewport at position.
    unsigned FindViewport(const IntVector2& mousePosition);

private:
    /// Graphics.
    Graphics& graphics_;
    /// Scene.
    Scene* scene_ = nullptr;
    /// Layout type.
    EditorViewportLayoutScheme layout_ = EditorViewportLayoutScheme::Empty;

    /// Viewports.
    Vector<SharedPtr<EditorViewport>> viewports_;
    /// Active viewport index.
    unsigned activeViewport_ = 0;
    /// Hovered viewport index.
    unsigned hoveredViewport_ = 0;
    /// Current camera ray.
    Ray currentCameraRay_;

};

}
