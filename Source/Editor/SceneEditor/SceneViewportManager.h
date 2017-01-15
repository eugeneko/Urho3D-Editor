#pragma once

#include "SceneOverlay.h"
#include <QSharedPointer>
#include <QVector>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3DEditor
{

class Configuration;
class SceneDocument;

struct SceneViewportUpdateParams
{
    /// Time step.
    float timeStep_;
    /// Input interface.
    SceneInputInterface* input_;
    /// Configuration.
    Configuration* config_;
    /// Specifies whether anything is selected.
    bool hasSelection_;
    /// Center of selection.
    Urho3D::Vector3 selectionCenter_;
};

/// Scene viewport description.
class SceneViewport
{
public:
    /// Construct.
    SceneViewport(Urho3D::Context* context, Urho3D::Scene* scene, Urho3D::Camera* camera);
    /// Set transform.
    void SetTransform(const Urho3D::Vector3& position, const Urho3D::Quaternion& rotation);
    /// Set rectangle.
    void SetRect(Urho3D::IntRect rect);
    /// Update viewport camera.
    void Update(const SceneViewportUpdateParams& p);

    /// Get camera node.
    Urho3D::Node& GetNode() { return cameraNode_; }
    /// Get viewport camera.
    Urho3D::Camera& GetCamera() const { return *viewportCamera_; }
    /// Get viewport.
    Urho3D::Viewport& GetViewport() const { return *viewport_; }

private:
    /// Viewport camera.
    Urho3D::Camera* sceneCamera_;
    /// Viewport rectangle.
    Urho3D::Rect rect_;

    /// Local camera node.
    Urho3D::Node cameraNode_;
    /// Local camera component.
    Urho3D::Camera& camera_;
    /// Local camera angles.
    Urho3D::Vector3 cameraAngles_;

    /// Controls whether in fly mode.
    bool flyMode_;
    /// Shows whether the camera is orbiting.
    bool orbiting_;

    /// Camera.
    Urho3D::Camera* viewportCamera_;
    /// Viewport.
    Urho3D::SharedPtr<Urho3D::Viewport> viewport_;

};

enum class SceneViewportLayout
{
    /// No viewports.
    Empty,
    /// Single viewport.
    Single,
    Vertical,
    Horizontal,
    Quad,
    Top1_Bottom2,
    Top2_Bottom1,
    Left1_Right2,
    Left2_Right1
};

class SceneViewportManager : public QObject, public Urho3D::Object, public SceneOverlay
{
    Q_OBJECT
    URHO3D_OBJECT(SceneViewportManager, Urho3D::Object);

public:
    /// Construct.
    SceneViewportManager(SceneDocument& document);
    /// Set layout.
    void SetLayout(SceneViewportLayout layout);
    /// Apply viewports to Urho3D Renderer.
    void ApplyViewports();

    /// Compute camera ray.
    Urho3D::Ray ComputeCameraRay(const Urho3D::Viewport& viewport, const Urho3D::IntVector2& mousePosition);
    /// Get current camera.
    Urho3D::Camera& GetCurrentCamera();
    /// Get current camera ray.
    const Urho3D::Ray& GetCurrentCameraRay() const { return currentCameraRay_; }

    /// General update.
    virtual void Update(SceneInputInterface& input, const Urho3D::Ray& cameraRay, float timeStep) override;

signals:
    /// Signals that viewports have been changed.
    void viewportsChanged();

private:
    /// Handle window resize.
    void HandleResize(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Update number of main viewports.
    void UpdateNumberOfViewports(int numViewports);
    /// Update current viewport.
    void SelectCurrentViewport(const Urho3D::IntVector2& mousePosition);
    /// Update viewport layout.
    void UpdateViewportLayout();

private:
    /// Document.
    SceneDocument& document_;
    /// Scene.
    Urho3D::Scene& scene_;
    /// Graphics.
    Urho3D::Graphics& graphics_;

    /// Viewports.
    QVector<QSharedPointer<SceneViewport>> viewports_;
    /// Current viewport index.
    int currentViewport_;
    /// Current camera ray.
    Urho3D::Ray currentCameraRay_;
    /// Layout type.
    SceneViewportLayout layout_;

};

}
