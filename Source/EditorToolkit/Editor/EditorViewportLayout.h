#pragma once

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
    /// Local camera angles.
    Vector3 cameraAngles_;

    /// Controls whether in fly mode.
    bool flyMode_;
    /// Shows whether the camera is orbiting.
    bool orbiting_;

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

class EditorViewportLayout : public Object
{
    URHO3D_OBJECT(EditorViewportLayout, Object);

public:
    /// Construct.
    EditorViewportLayout(Context* context);
    /// Set scene.
    void SetScene(Scene* scene);
    /// Set camera for each viewport.
    void SetCamera(Node* cameraNode);
    /// Set layout.
    void SetLayout(EditorViewportLayoutScheme layout);
    /// Apply viewports to Urho3D Renderer.
    void ApplyViewports();

    /// Compute camera ray.
    Ray ComputeCameraRay(const Viewport& viewport, const IntVector2& mousePosition);
    /// Get current camera.
    Camera& GetCurrentCamera();
    /// Get current camera ray.
    const Ray& GetCurrentCameraRay() const { return currentCameraRay_; }

private:
    /// Handle window resize.
    void HandleResize(StringHash eventType, VariantMap& eventData);
    /// Update number of main viewports.
    void UpdateNumberOfViewports(int numViewports);
    /// Update current viewport.
    void SelectCurrentViewport(const IntVector2& mousePosition);
    /// Update viewport layout.
    void UpdateViewportLayout();

private:
    /// Graphics.
    Graphics& graphics_;
    /// Scene.
    Scene* scene_ = nullptr;

    /// Viewports.
    Vector<SharedPtr<EditorViewport>> viewports_;
    /// Current viewport index.
    int currentViewport_;
    /// Current camera ray.
    Ray currentCameraRay_;
    /// Layout type.
    EditorViewportLayoutScheme layout_;

};

}
