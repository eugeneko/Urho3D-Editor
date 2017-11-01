#include "Gizmo.h"
#include "Transformable.h"
#include "../AbstractUI/AbstractInput.h"
// #include "Transformable.h"
// #include "SceneActions.h"
// #include "SceneDocument.h"
// #include "SceneEditor.h"
// #include "../Configuration.h"
// #include "../Core/Core.h"
// #include "../Core/QtUrhoHelpers.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>

namespace Urho3D
{

namespace
{

/// Snap vector to grid.
void SnapVector(Vector3& vector, float step)
{
    vector.x_ = Round(vector.x_ / step) * step;
    vector.y_ = Round(vector.y_ / step) * step;
    vector.z_ = Round(vector.z_ / step) * step;
}

}

//////////////////////////////////////////////////////////////////////////
void GizmoAxis::Update(Ray cameraRay, float scale, bool drag, float axisMaxT, float axisMaxD)
{
    using namespace Urho3D;

    Vector3 closest = cameraRay.ClosestPoint(axisRay);
    Vector3 projected = axisRay.Project(closest);
    d = axisRay.Distance(closest);
    t = (projected - axisRay.origin_).DotProduct(axisRay.direction_);

    // Determine the sign of d from a plane that goes through the camera position to the axis
    // #TODO Make sure that cameraNode.position -> cameraRay.origin is correct
    Plane axisPlane(cameraRay.origin_, axisRay.origin_, axisRay.origin_ + axisRay.direction_);
    if (axisPlane.Distance(closest) < 0.0)
        d = -d;

    // Update selected status only when not dragging
    if (!drag)
    {
        selected = Abs(d) < axisMaxD * scale && t >= -axisMaxD * scale && t <= axisMaxT * scale;
        lastT = t;
        lastD = d;
    }
}

void GizmoAxis::Moved()
{
    lastT = t;
    lastD = d;
}

//////////////////////////////////////////////////////////////////////////
Gizmo::Gizmo(Context* context)
    : AbstractEditorOverlay(context)
    , gizmoNode_(context_)
    , gizmo_(*gizmoNode_.CreateComponent<StaticModel>())
    , mouseDrag_(false)
    , lastMouseDrag_(false)
    , keyDrag_(false)
    , lastKeyDrag_(false)
    , moved_(false)
{
    // Setup gizmo
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    gizmo_.SetModel(GetGizmoModel(GizmoType::Position));
    gizmo_.SetMaterial(0, GetGizmoMaterial(0, false));
    gizmo_.SetMaterial(1, GetGizmoMaterial(1, false));
    gizmo_.SetMaterial(2, GetGizmoMaterial(2, false));
    gizmo_.SetEnabled(false);
    gizmo_.SetViewMask(0x80000000);
    gizmo_.SetOccludee(false);
    gizmoNode_.SetName("EditorGizmo");
}

void Gizmo::SetGizmoType(GizmoType type, float step /*= 1.0f*/, float snapScale /*= 0.0f*/)
{
    if (gizmoType_ != type)
    {
        gizmo_.SetModel(GetGizmoModel(type));
        gizmoType_ = type;
    }
    step_ = step;
    snapScale_ = snapScale;
}

void Gizmo::Update(AbstractInput& input, AbstractEditorContext& editorContext, float timeStep)
{
    if (!transformable_)
        return;
    UpdateDragState(input);
    PrepareUndo();
    UseGizmoKeyboard(input, timeStep);
    if (UseGizmoMouse(editorContext.GetMouseRay()))
    {
        input.GrabMouseButton(MOUSEB_LEFT);
        input.GrabMouseMove();
    }
    FinalizeUndo();
    PositionGizmo();
    ResizeGizmo(editorContext);
}

Model* Gizmo::GetGizmoModel(GizmoType type)
{
    String modelName;
    switch (type)
    {
    case GizmoType::Select:
    case GizmoType::Position:
        modelName = "Models/Editor/Axes.mdl";
        break;
    case GizmoType::Rotation:
        modelName = "Models/Editor/RotateAxes.mdl";
        break;
    case GizmoType::Scale:
        modelName = "Models/Editor/ScaleAxes.mdl";
        break;
    default:
        assert(0);
        return nullptr;
    }

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    return cache->GetResource<Model>(modelName);
}

Material* Gizmo::GetGizmoMaterial(int axis, bool highlight)
{
    String materialName;
    switch (axis)
    {
    case 0:
        materialName = highlight ? "Materials/Editor/BrightRedUnlit.xml" : "Materials/Editor/RedUnlit.xml";
        break;
    case 1:
        materialName = highlight ? "Materials/Editor/BrightGreenUnlit.xml" : "Materials/Editor/GreenUnlit.xml";
        break;
    case 2:
        materialName = highlight ? "Materials/Editor/BrightBlueUnlit.xml" : "Materials/Editor/BlueUnlit.xml";
        break;
    default:
        assert(0);
        return nullptr;
    }

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    return cache->GetResource<Material>(materialName);
}

void Gizmo::ShowGizmo()
{
    gizmo_.SetEnabled(true);
    if (Octree* octree = transformable_->GetScene().GetComponent<Octree>())
        octree->AddManualDrawable(&gizmo_);
}

void Gizmo::HideGizmo()
{
    gizmo_.SetEnabled(false);
}

void Gizmo::UpdateDragState(AbstractInput& input)
{
    // Update mouse drag state
    mouseDrag_ = input.IsMouseButtonDown(MOUSEB_LEFT)
        && !input.IsMouseButtonGrabbed(MOUSEB_LEFT) && !input.IsMouseMoveGrabbed();

    // Update keyboard drag state
    keyDrag_ = false;
    if (input.IsKeyDown(KEY_CTRL))
    {
        if (input.IsKeyDown(KEY_UP) || input.IsKeyDown(KEY_DOWN)
            || input.IsKeyDown(KEY_LEFT) || input.IsKeyDown(KEY_RIGHT)
            || input.IsKeyDown(KEY_PAGEUP) || input.IsKeyDown(KEY_PAGEDOWN))
        {
            keyDrag_ = true;
        }
        if (gizmoType_ == GizmoType::Scale
            && (input.IsKeyDown(KEY_KP_PLUS) || input.IsKeyDown(KEY_KP_MINUS)))
        {
            keyDrag_ = true;
        }
    }
}

void Gizmo::PrepareUndo()
{
    if (!gizmo_.IsEnabled() || gizmoType_ == GizmoType::Select)
    {
        FlushActions();
        lastMouseDrag_ = false;
        lastKeyDrag_ = false;
    }

    if (gizmoType_ == GizmoType::Select)
        return;

    // Store initial transforms for undo when gizmo drag started
    // #TODO
//     if (IsDragging() && !WasDragging())
//     {
//         editNodes_ = document_.GetSelectedNodesAndComponents().toList();
//         oldTransforms_.resize(editNodes_.size());
//         for (int i = 0; i < editNodes_.size(); ++i)
//             oldTransforms_[i].Define(*editNodes_[i]);
//     }
}

void Gizmo::FinalizeUndo()
{
    if (!IsDragging() && WasDragging())
        FlushActions();

    lastKeyDrag_ = keyDrag_;
    lastMouseDrag_ = mouseDrag_;
}

void Gizmo::PositionGizmo()
{
    using namespace Urho3D;

//     Configuration& config = document_.GetConfig();
//     const GizmoType type = (GizmoType)config.GetValue(SceneEditor::VarGizmoType).toInt();
//     const GizmoAxisMode axisMode = (GizmoAxisMode)config.GetValue(SceneEditor::VarGizmoAxisMode).toInt();
//
//     // Gather nodes. Hide gizmo if scene is selected.
//     const SceneDocument::NodeSet& editNodes = document_.GetSelectedNodesAndComponents();
//     const bool containsScene = editNodes.contains(&document_.GetScene());
    if (transformable_->IsEmpty())
    {
        HideGizmo();
        return;
    }

    // Setup position
    gizmoNode_.SetPosition(transformable_->GetPosition());

    // Setup rotation
    if (axisMode_ == GizmoAxisMode::World)
        gizmoNode_.SetRotation(Quaternion::IDENTITY);
    else
        gizmoNode_.SetRotation(transformable_->GetRotation());

    // Show or hide
    bool orbiting = false; // #TODO Add support for this stuff
    if ((gizmoType_ != GizmoType::Select && !orbiting) && !gizmo_.IsEnabled())
        ShowGizmo();
    else if ((gizmoType_ == GizmoType::Select || orbiting) && gizmo_.IsEnabled())
        HideGizmo();
}

void Gizmo::ResizeGizmo(AbstractEditorContext& editorContext)
{
    if (!gizmo_.IsEnabled())
        return;

    const Camera& camera = *editorContext.GetCurrentCamera();
    float scale = 0.1f / camera.GetZoom();

    if (camera.IsOrthographic())
        scale *= camera.GetOrthoSize();
    else
        scale *= (camera.GetView() * gizmoNode_.GetPosition()).z_;

    gizmoNode_.SetScale(scale);
}

void Gizmo::CalculateGizmoAxes()
{
    using namespace Urho3D;
    axisX_.axisRay = Ray(gizmoNode_.GetPosition(), gizmoNode_.GetRotation() * Vector3(1, 0, 0));
    axisY_.axisRay = Ray(gizmoNode_.GetPosition(), gizmoNode_.GetRotation() * Vector3(0, 1, 0));
    axisZ_.axisRay = Ray(gizmoNode_.GetPosition(), gizmoNode_.GetRotation() * Vector3(0, 0, 1));
}

void Gizmo::MarkMoved()
{
    axisX_.Moved();
    axisY_.Moved();
    axisZ_.Moved();
    moved_ = true;
    OnChanged();
    // #TODO
//     for (Node* node : editNodes_)
//         emit document_.nodeTransformChanged(*node);
}

void Gizmo::FlushActions()
{
    // #TODO:
//     if (moved_ && !editNodes_.isEmpty() && editNodes_.size() == oldTransforms_.size())
//     {
//         QScopedPointer<QUndoCommand> group(new QUndoCommand("Transform"));
//         for (int i = 0; i < editNodes_.size(); ++i)
//             new EditNodeTransformAction(document_, *editNodes_[i], oldTransforms_[i], group.data());
//         document_.AddAction(group.take());
//     }

    moved_ = false;
}

void Gizmo::UseGizmoKeyboard(AbstractInput& input, float timeStep)
{
    using namespace Urho3D;

    if (transformable_->IsEmpty() || gizmoType_ == GizmoType::Select)
        return;

//     Configuration& config = document_.GetConfig();
//     const HotKeyMode hotKeyMode = (HotKeyMode)config.GetValue(SceneEditor::VarHotKeyMode).toInt();
//     const GizmoType gizmoType_ = (GizmoType)config.GetValue(SceneEditor::VarGizmoType).toInt();
//     const float moveStep = config.GetValue(SceneEditor::VarSnapPositionStep).toFloat();
//     const bool moveSnap = config.GetValue(SceneEditor::VarSnapPosition).toBool();
//     const float rotateStep = config.GetValue(SceneEditor::VarSnapRotationStep).toFloat();
//     const bool rotateSnap = config.GetValue(SceneEditor::VarSnapRotation).toBool();
//     const float scaleStep = config.GetValue(SceneEditor::VarSnapScaleStep).toFloat();
//     const bool scaleSnap = config.GetValue(SceneEditor::VarSnapScale).toBool();

    // Process continuous movement
    if (!input.IsKeyDown(KEY_CTRL))
        return;

    Vector3 adjust(0, 0, 0);
    if (input.IsKeyDown(KEY_UP))
        adjust.z_ = 1;
    if (input.IsKeyDown(KEY_DOWN))
        adjust.z_ = -1;
    if (input.IsKeyDown(KEY_LEFT))
        adjust.x_ = -1;
    if (input.IsKeyDown(KEY_RIGHT))
        adjust.x_ = 1;
    if (input.IsKeyDown(KEY_PAGEUP))
        adjust.y_ = 1;
    if (input.IsKeyDown(KEY_PAGEDOWN))
        adjust.y_ = -1;
    if (gizmoType_ == GizmoType::Scale)
    {
        if (input.IsKeyDown(KEY_KP_PLUS))
            adjust = Vector3(1, 1, 1);
        if (input.IsKeyDown(KEY_KP_MINUS))
            adjust = Vector3(-1, -1, -1);
    }

    if (adjust != Vector3::ZERO)
    {
        bool moved = false;
        adjust *= timeStep * 10;

        switch (gizmoType_)
        {
        case GizmoType::Position:
            if (!IsSnapped())
                moved = MoveNodes(adjust);
            break;

        case GizmoType::Rotation:
            if (!IsSnapped())
                moved = RotateNodes(adjust);
            break;

        case GizmoType::Scale:
            if (!IsSnapped())
                moved = ScaleNodes(adjust);
            break;
        }

        if (moved)
            MarkMoved();
    }

    // Process snapped movement
    adjust = Vector3::ZERO;
    if (input.IsKeyPressed(KEY_UP))
        adjust.z_ = 1;
    if (input.IsKeyPressed(KEY_DOWN))
        adjust.z_ = -1;
    if (input.IsKeyPressed(KEY_LEFT))
        adjust.x_ = -1;
    if (input.IsKeyPressed(KEY_RIGHT))
        adjust.x_ = 1;
    if (input.IsKeyPressed(KEY_PAGEUP))
        adjust.y_ = 1;
    if (input.IsKeyPressed(KEY_PAGEDOWN))
        adjust.y_ = -1;
    if (gizmoType_ == GizmoType::Scale)
    {
        if (input.IsKeyPressed(KEY_KP_PLUS))
            adjust = Vector3(1, 1, 1);
        if (input.IsKeyPressed(KEY_KP_MINUS))
            adjust = Vector3(-1, -1, -1);
    }

    if (adjust != Vector3::ZERO)
    {
        bool moved = false;

        switch (gizmoType_)
        {
        case GizmoType::Position:
            if (IsSnapped())
                moved = MoveNodes(adjust);
            break;

        case GizmoType::Rotation:
            if (IsSnapped())
                moved = RotateNodes(adjust * step_ * snapScale_);
            break;

        case GizmoType::Scale:
            if (IsSnapped())
                moved = ScaleNodes(adjust * step_ * snapScale_);
            break;
        }

        if (moved)
            MarkMoved();
    }
}

bool Gizmo::UseGizmoMouse(const Ray& mouseRay)
{
    using namespace Urho3D;

    const float scale = gizmoNode_.GetScale().x_;

    // Recalculate axes only when not left-dragging
    if (!mouseDrag_)
        CalculateGizmoAxes();

    // #TODO Move to config
    const float axisMaxD = 0.1f;
    const float axisMaxT = 1.0f;
    const float rotSensitivity = 50.0f;

    axisX_.Update(mouseRay, scale, mouseDrag_, axisMaxT, axisMaxD);
    axisY_.Update(mouseRay, scale, mouseDrag_, axisMaxT, axisMaxD);
    axisZ_.Update(mouseRay, scale, mouseDrag_, axisMaxT, axisMaxD);

    if (axisX_.selected != axisX_.lastSelected)
    {
        gizmo_.SetMaterial(0, GetGizmoMaterial(0, axisX_.selected));
        axisX_.lastSelected = axisX_.selected;
    }
    if (axisY_.selected != axisY_.lastSelected)
    {
        gizmo_.SetMaterial(1, GetGizmoMaterial(1, axisY_.selected));
        axisY_.lastSelected = axisY_.selected;
    }
    if (axisZ_.selected != axisZ_.lastSelected)
    {
        gizmo_.SetMaterial(2, GetGizmoMaterial(2, axisZ_.selected));
        axisZ_.lastSelected = axisZ_.selected;
    };

    if (mouseDrag_)
    {
        // Store initial transforms for undo when gizmo drag started
        if (!lastMouseDrag_)
            transformable_->StartTransformation();

        bool moved = false;

        if (gizmoType_ == GizmoType::Position)
        {
            Vector3 adjust(0, 0, 0);
            if (axisX_.selected)
                adjust += Vector3(1, 0, 0) * (axisX_.t - axisX_.lastT);
            if (axisY_.selected)
                adjust += Vector3(0, 1, 0) * (axisY_.t - axisY_.lastT);
            if (axisZ_.selected)
                adjust += Vector3(0, 0, 1) * (axisZ_.t - axisZ_.lastT);

            moved = MoveNodes(adjust);
        }
        else if (gizmoType_ == GizmoType::Rotation)
        {
            Vector3 adjust(0, 0, 0);
            if (axisX_.selected)
                adjust.x_ = (axisX_.d - axisX_.lastD) * rotSensitivity / scale;
            if (axisY_.selected)
                adjust.y_ = -(axisY_.d - axisY_.lastD) * rotSensitivity / scale;
            if (axisZ_.selected)
                adjust.z_ = (axisZ_.d - axisZ_.lastD) * rotSensitivity / scale;

            moved = RotateNodes(adjust);
        }
        else if (gizmoType_ == GizmoType::Scale)
        {
            Vector3 adjust(0, 0, 0);
            if (axisX_.selected)
                adjust += Vector3(1, 0, 0) * (axisX_.t - axisX_.lastT);
            if (axisY_.selected)
                adjust += Vector3(0, 1, 0) * (axisY_.t - axisY_.lastT);
            if (axisZ_.selected)
                adjust += Vector3(0, 0, 1) * (axisZ_.t - axisZ_.lastT);

            // Special handling for uniform scale: use the unmodified X-axis movement only
            if (gizmoType_ == GizmoType::Scale && axisX_.selected && axisY_
                .selected && axisZ_.selected)
            {
                float x = axisX_.t - axisX_.lastT;
                adjust = Vector3(x, x, x);
            }

            moved = ScaleNodes(adjust);
        }

        if (moved)
            MarkMoved();
    }
    else
    {
        if (lastMouseDrag_)
            FlushActions();
    }

    lastMouseDrag_ = mouseDrag_;

    return (axisX_.selected || axisY_.selected || axisZ_.selected)
        && gizmoType_ != GizmoType::Select && gizmo_.IsEnabled();
}

bool Gizmo::MoveNodes(Vector3 adjust)
{
    // Apply snap
    if (IsSnapped())
        SnapVector(adjust, step_ * snapScale_);

    if (adjust.Length() < M_EPSILON)
        return false;

    // Apply local space
    if (axisMode_ == GizmoAxisMode::Local)
        adjust = transformable_->GetRotation() * adjust;

    // Apply transform
    transformable_->ApplyPositionChange(adjust);
    return true;
}

bool Gizmo::RotateNodes(Vector3 adjust)
{
    // Apply snap
    if (IsSnapped())
        SnapVector(adjust, step_ * snapScale_);

    if (adjust.Length() < M_EPSILON)
        return false;

    // Apply local space
    Quaternion rotationDelta(adjust.x_, adjust.y_, adjust.z_);
    if (axisMode_ == GizmoAxisMode::Local)
    {
        const Quaternion globalRotation = transformable_->GetRotation();
        rotationDelta = globalRotation * rotationDelta * globalRotation.Inverse();
    }

    // Apply transform
    transformable_->ApplyRotationChange(rotationDelta);
    return true;
}

bool Gizmo::ScaleNodes(Vector3 adjust)
{
    if (adjust.Length() < M_EPSILON)
        return false;

    // Apply transform
    transformable_->ApplyScaleChange(adjust);
    if (IsSnapped())
        transformable_->SnapScale(step_ * snapScale_);
    return true;
}

void Gizmo::OnChanged()
{
    if (onChanged_)
        onChanged_();
}

}
