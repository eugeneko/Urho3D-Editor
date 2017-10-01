#include "EditorViewportLayout.h"
// #include "SceneDocument.h"
// #include "SceneEditor.h"
// #include "../Configuration.h"
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>

namespace Urho3D
{

/// Get number of viewports.
int GetNumberOfViewports(EditorViewportLayoutScheme layout)
{
    switch (layout)
    {
    case EditorViewportLayoutScheme::Single:
        return 1;
    case EditorViewportLayoutScheme::Vertical:
    case EditorViewportLayoutScheme::Horizontal:
        return 2;
    case EditorViewportLayoutScheme::Top1_Bottom2:
    case EditorViewportLayoutScheme::Top2_Bottom1:
    case EditorViewportLayoutScheme::Left1_Right2:
    case EditorViewportLayoutScheme::Left2_Right1:
        return 3;
    case EditorViewportLayoutScheme::Quad:
        return 4;
    case EditorViewportLayoutScheme::Empty:
    default:
        return 0;
    }
}

//////////////////////////////////////////////////////////////////////////
EditorViewport::EditorViewport(Context* context, Scene* scene, Camera* camera)
    : sceneCamera_(camera)
    , cameraNode_(context)
    , camera_(*cameraNode_.CreateComponent<Camera>())
    , viewportCamera_(sceneCamera_ ? sceneCamera_ : &camera_)
    , viewport_(new Viewport(context, scene, viewportCamera_))
{
}

void EditorViewport::SetTransform(const Vector3& position, const Quaternion& rotation)
{
    cameraNode_.SetWorldPosition(position);
    cameraNode_.SetWorldRotation(rotation);
}

void EditorViewport::SetRect(IntRect rect)
{
    viewport_->SetRect(rect);
}

//////////////////////////////////////////////////////////////////////////
EditorViewportLayout::EditorViewportLayout(Context* context)
    : AbstractEditorOverlay(context)
    , graphics_(*GetSubsystem<Graphics>())
{
    SetLayout(EditorViewportLayoutScheme::Single); // #TODO Change
    SubscribeToEvent(E_SCREENMODE, URHO3D_HANDLER(EditorViewportLayout, HandleResize));
}

void EditorViewportLayout::Update(AbstractEditorInput& input, float timeStep)
{
    const bool isAnyMouseButtonPressed =
        input.IsMouseButtonPressed(MOUSEB_LEFT)
        || input.IsMouseButtonPressed(MOUSEB_RIGHT)
        || input.IsMouseButtonPressed(MOUSEB_MIDDLE);

    // Update current viewport
    if (isAnyMouseButtonPressed)
        UpdateActiveViewport(input.GetMousePosition());
    UpdateHoveredViewport(input.GetMousePosition());

    // Update ray
    currentCameraRay_ = ComputeCameraRay(viewports_[hoveredViewport_]->GetViewport(), input.GetMousePosition());
}

void EditorViewportLayout::SetScene(Scene* scene)
{
    scene_ = scene;
    UpdateViewports();
}

void EditorViewportLayout::SetCameraTransform(Node* cameraNode)
{
    for (EditorViewport* viewport : viewports_)
        viewport->SetTransform(cameraNode->GetWorldPosition(), cameraNode->GetWorldRotation());
}

void EditorViewportLayout::SetLayout(EditorViewportLayoutScheme layout)
{
    layout_ = layout;
    UpdateViewports();
}

Ray EditorViewportLayout::ComputeCameraRay(const Viewport& viewport, const IntVector2& mousePosition) const
{
    using namespace Urho3D;

    const IntRect rect = viewport.GetRect().Size() == IntVector2::ZERO
        ? IntRect(0, 0, graphics_.GetWidth(), graphics_.GetHeight())
        : viewport.GetRect();

    return viewport.GetCamera()->GetScreenRay(
        float(mousePosition.x_ - rect.left_) / rect.Width(),
        float(mousePosition.y_ - rect.top_) / rect.Height());
}

Camera& EditorViewportLayout::GetCurrentCamera()
{
    return viewports_[activeViewport_]->GetCamera();
}

void EditorViewportLayout::HandleResize(StringHash eventType, VariantMap& eventData)
{
    UpdateViewportsSize();
}

void EditorViewportLayout::UpdateViewports()
{
    if (!scene_)
        return;

    using namespace Urho3D;

    Renderer* renderer = GetSubsystem<Renderer>();

    // Get old viewports
    Vector<Viewport*> oldViewports;
    for (unsigned i = 0; i < renderer->GetNumViewports(); ++i)
        oldViewports.Push(renderer->GetViewport(i));

    // Remove old layout viewports
    for (EditorViewport* viewport : viewports_)
        oldViewports.Remove(&viewport->GetViewport());

    // Get default transform of new viewports
    static const Vector3 defaultPosition(0, 10, -10);
    static const Quaternion defaultRotation(45, 0, 0);
    Vector3 position = defaultPosition;
    Quaternion rotation = defaultRotation;
    if (!viewports_.Empty())
    {
        Node& node = viewports_.Back()->GetNode();
        position = node.GetWorldPosition();
        rotation = node.GetWorldRotation();
    }

    // Re-allocate viewports
    const unsigned numViewports = GetNumberOfViewports(layout_);
    if (numViewports < viewports_.Size())
        viewports_.Resize(numViewports);
    else
    {
        while (viewports_.Size() < numViewports)
        {
            auto viewport = MakeShared<EditorViewport>(context_, scene_, nullptr);
            viewport->SetTransform(position, rotation);
            viewports_.Push(viewport);
        }
    }

    // Append viewports to array
    for (EditorViewport* viewport : viewports_)
        oldViewports.Push(&viewport->GetViewport());

    // Set viewports
    renderer->SetNumViewports(oldViewports.Size());
    for (unsigned i = 0; i < oldViewports.Size(); ++i)
        renderer->SetViewport(i, oldViewports[i]);

    // Update layout
    UpdateViewportsSize();
}

void EditorViewportLayout::UpdateViewportsSize()
{
    if (!scene_)
        return;

    using namespace Urho3D;
    Graphics* graphics = GetSubsystem<Graphics>();
    const int width = graphics->GetWidth();
    const int height = graphics->GetHeight();
    const int halfWidth = width / 2;
    const int halfHeight = height / 2;

    switch (layout_)
    {
    case EditorViewportLayoutScheme::Empty:
        break;
    case EditorViewportLayoutScheme::Single:
        viewports_[0]->SetRect(IntRect(0, 0, width, height));
        break;
    case EditorViewportLayoutScheme::Vertical:
        viewports_[0]->SetRect(IntRect(0, 0, halfWidth, height));
        viewports_[1]->SetRect(IntRect(halfWidth, 0, width, height));
        break;
    case EditorViewportLayoutScheme::Horizontal:
        viewports_[0]->SetRect(IntRect(0, 0, width, halfHeight));
        viewports_[1]->SetRect(IntRect(0, halfHeight, width, height));
        break;
    case EditorViewportLayoutScheme::Quad:
        viewports_[0]->SetRect(IntRect(0, 0, halfWidth, halfHeight));
        viewports_[1]->SetRect(IntRect(halfWidth, 0, width, halfHeight));
        viewports_[2]->SetRect(IntRect(0, halfHeight, halfWidth, height));
        viewports_[3]->SetRect(IntRect(halfWidth, halfHeight, width, height));
        break;
    case EditorViewportLayoutScheme::Top1_Bottom2:
        viewports_[0]->SetRect(IntRect(0, 0, width, halfHeight));
        viewports_[1]->SetRect(IntRect(0, halfHeight, halfWidth, height));
        viewports_[2]->SetRect(IntRect(halfWidth, halfHeight, width, height));
        break;
    case EditorViewportLayoutScheme::Top2_Bottom1:
        viewports_[0]->SetRect(IntRect(0, 0, halfWidth, halfHeight));
        viewports_[1]->SetRect(IntRect(halfWidth, 0, width, halfHeight));
        viewports_[2]->SetRect(IntRect(0, halfHeight, width, height));
        break;
    case EditorViewportLayoutScheme::Left1_Right2:
        viewports_[0]->SetRect(IntRect(0, 0, halfWidth, height));
        viewports_[1]->SetRect(IntRect(halfWidth, 0, width, halfHeight));
        viewports_[2]->SetRect(IntRect(halfWidth, halfHeight, width, height));
        break;
    case EditorViewportLayoutScheme::Left2_Right1:
        viewports_[0]->SetRect(IntRect(0, 0, halfWidth, halfHeight));
        viewports_[1]->SetRect(IntRect(0, halfHeight, halfWidth, height));
        viewports_[2]->SetRect(IntRect(halfWidth, 0, width, height));
        break;
    default:
        break;
    }
}

void EditorViewportLayout::UpdateActiveViewport(const IntVector2& mousePosition)
{
    activeViewport_ = FindViewport(mousePosition);

    // Send event.
    SendEvent(E_EDITORCURRENTVIEWPORTCHANGED,
        EditorCurrentViewportChanged::P_VIEWPORTLAYOUT, this,
        EditorCurrentViewportChanged::P_CAMERA, &viewports_[activeViewport_]->GetCamera());
}

void EditorViewportLayout::UpdateHoveredViewport(const IntVector2& mousePosition)
{
    hoveredViewport_ = FindViewport(mousePosition);
}

unsigned EditorViewportLayout::FindViewport(const IntVector2& mousePosition)
{
    for (unsigned i = 0; i < viewports_.Size(); ++i)
    {
        const Viewport& viewport = viewports_[i]->GetViewport();
        const IntRect rect = viewport.GetRect();
        if (rect.Size() == IntVector2::ZERO || rect.IsInside(mousePosition) != OUTSIDE)
            return i;
    }
    return viewports_.Size();
}

}
