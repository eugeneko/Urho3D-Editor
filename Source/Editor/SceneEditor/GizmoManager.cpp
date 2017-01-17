#include "GizmoManager.h"
#include "SceneActions.h"
#include "SceneDocument.h"
#include "SceneEditor.h"
#include "../Configuration.h"
#include "../MainWindow.h"
#include "../Bridge.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Resource/ResourceCache.h>

namespace Urho3DEditor
{

//////////////////////////////////////////////////////////////////////////
GizmoAxis::GizmoAxis()
    : selected(false)
    , lastSelected(false)
    , t(0.0)
    , d(0.0)
    , lastT(0.0)
    , lastD(0.0)
{
}

void GizmoAxis::Update(Urho3D::Ray cameraRay, float scale, bool drag, float axisMaxT, float axisMaxD)
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
Gizmo::Gizmo(SceneDocument& document)
    : document_(document)
    , gizmoNode_(document.GetContext())
    , gizmo_(*gizmoNode_.CreateComponent<Urho3D::StaticModel>())
    , lastType_(GizmoType::Position)
    , mouseDrag_(false)
    , lastMouseDrag_(false)
    , keyDrag_(false)
    , lastKeyDrag_(false)
    , moved_(false)
{
    document_.AddOverlay(this);

    // Setup gizmo
    Urho3D::ResourceCache* cache = document.GetContext()->GetSubsystem<Urho3D::ResourceCache>();
    gizmo_.SetModel(GetGizmoModel(GizmoType::Position));
    gizmo_.SetMaterial(0, GetGizmoMaterial(0, false));
    gizmo_.SetMaterial(1, GetGizmoMaterial(1, false));
    gizmo_.SetMaterial(2, GetGizmoMaterial(2, false));
    gizmo_.SetEnabled(false);
    gizmo_.SetViewMask(0x80000000);
    gizmo_.SetOccludee(false);
    gizmoNode_.SetName("EditorGizmo");
}

Gizmo::~Gizmo()
{
}

void Gizmo::Update(SceneInputInterface& input, float timeStep)
{
    UpdateDragState(input);
    PrepareUndo();
    UseGizmoKeyboard(input, timeStep);
    UseGizmoMouse(input.GetMouseRay());
    FinalizeUndo();
    PositionGizmo();
    ResizeGizmo();
}

Urho3D::Model* Gizmo::GetGizmoModel(GizmoType type)
{
    Configuration& config = document_.GetConfig();

    QString variableName;
    switch (type)
    {
    case GizmoType::Select:
    case GizmoType::Position:
        variableName = SceneEditor::VarModelPosition;
        break;
    case GizmoType::Rotation:
        variableName = SceneEditor::VarModelRotation;
        break;
    case GizmoType::Scale:
        variableName = SceneEditor::VarModelScale;
        break;
    default:
        Q_ASSERT(0);
        return nullptr;
    }

    Urho3D::ResourceCache* cache = document_.GetSubsystem<Urho3D::ResourceCache>();
    const QString modelName = config.GetValue(variableName).toString();
    return cache->GetResource<Urho3D::Model>(Cast(modelName));
}

Urho3D::Material* Gizmo::GetGizmoMaterial(int axis, bool highlight)
{
    Configuration& config = document_.GetConfig();

    QString variableName;
    switch (axis)
    {
    case 0:
        variableName = highlight ? SceneEditor::VarMaterialRedHighlight : SceneEditor::VarMaterialRed;
        break;
    case 1:
        variableName = highlight ? SceneEditor::VarMaterialGreenHighlight : SceneEditor::VarMaterialGreen;
        break;
    case 2:
        variableName = highlight ? SceneEditor::VarMaterialBlueHighlight : SceneEditor::VarMaterialBlue;
        break;
    default:
        Q_ASSERT(0);
        return nullptr;
    }

    Urho3D::ResourceCache* cache = document_.GetSubsystem<Urho3D::ResourceCache>();
    const QString materialName = config.GetValue(variableName).toString();
    return cache->GetResource<Urho3D::Material>(Cast(materialName));
}

void Gizmo::ShowGizmo()
{
    gizmo_.SetEnabled(true);
    if (Urho3D::Octree* octree = document_.GetScene().GetComponent<Urho3D::Octree>())
        octree->AddManualDrawable(&gizmo_);
}

void Gizmo::HideGizmo()
{
    gizmo_.SetEnabled(false);
}

void Gizmo::UpdateDragState(SceneInputInterface& input)
{
    // Update mouse drag state
    mouseDrag_ = input.IsMouseButtonDown(Qt::LeftButton);

    // Update keyboard drag state
    Configuration& config = document_.GetConfig();
    const GizmoType editMode = (GizmoType)config.GetValue(SceneEditor::VarGizmoType).toInt();
    const bool moveSnap = config.GetValue(SceneEditor::VarSnapPosition).toBool();
    const bool rotateSnap = config.GetValue(SceneEditor::VarSnapRotation).toBool();
    const bool scaleSnap = config.GetValue(SceneEditor::VarSnapScale).toBool();

    keyDrag_ = false;
    if (input.IsKeyDown(Qt::Key_Control))
    {
        if (input.IsKeyDown(Qt::Key_Up) || input.IsKeyDown(Qt::Key_Down)
            || input.IsKeyDown(Qt::Key_Left) || input.IsKeyDown(Qt::Key_Right)
            || input.IsKeyDown(Qt::Key_PageUp) || input.IsKeyDown(Qt::Key_PageDown))
        {
            keyDrag_ = true;
        }
        if (editMode == GizmoType::Scale
            && (input.IsKeyDown(Qt::Key_Plus) || input.IsKeyDown(Qt::Key_Minus)))
        {
            keyDrag_ = true;
        }
    }
}

void Gizmo::PrepareUndo()
{
    Configuration& config = document_.GetConfig();
    const GizmoType editMode = (GizmoType)config.GetValue(SceneEditor::VarGizmoType).toInt();

    if (!gizmo_.IsEnabled() || editMode == GizmoType::Select)
    {
        FlushActions();
        lastMouseDrag_ = false;
        lastKeyDrag_ = false;
    }

    if (editMode == GizmoType::Select)
        return;

    // Store initial transforms for undo when gizmo drag started
    if (IsDragging() && !WasDragging())
    {
        editNodes_ = document_.GetSelectedNodesAndComponents().toList();
        oldTransforms_.resize(editNodes_.size());
        for (int i = 0; i < editNodes_.size(); ++i)
            oldTransforms_[i].Define(*editNodes_[i]);
    }
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

    Configuration& config = document_.GetConfig();
    const GizmoType type = (GizmoType)config.GetValue(SceneEditor::VarGizmoType).toInt();
    const GizmoAxisMode axisMode = (GizmoAxisMode)config.GetValue(SceneEditor::VarGizmoAxisMode).toInt();

    // Gather nodes. Hide gizmo if scene is selected.
    const SceneDocument::NodeSet& editNodes = document_.GetSelectedNodesAndComponents();
    const bool containsScene = editNodes.contains(&document_.GetScene());
    if (containsScene || editNodes.isEmpty())
    {
        HideGizmo();
        return;
    }

    // Setup position
    Vector3 center;
    for (Node* node : editNodes)
        center += node->GetWorldPosition();
    center /= editNodes.size();
    gizmoNode_.SetPosition(center);

    // Setup rotation
    if (axisMode == GizmoAxisMode::World || editNodes.size() > 1)
        gizmoNode_.SetRotation(Quaternion());
    else
        gizmoNode_.SetRotation((**editNodes.begin()).GetWorldRotation());

    // Setup model
    if (type != lastType_)
    {
        gizmo_.SetModel(GetGizmoModel(type));
        lastType_ = type;
    }

    // Show or hide
    bool orbiting = false; // #TODO Add support for this stuff
    if ((type != GizmoType::Select && !orbiting) && !gizmo_.IsEnabled())
        ShowGizmo();
    else if ((type == GizmoType::Select || orbiting) && gizmo_.IsEnabled())
        HideGizmo();
}

void Gizmo::ResizeGizmo()
{
    if (!gizmo_.IsEnabled())
        return;

    const Urho3D::Camera& camera = document_.GetCurrentCamera();
    float scale = 0.1 / camera.GetZoom();

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
    for (Urho3D::Node* node : editNodes_)
        emit document_.nodeTransformChanged(*node);
}

void Gizmo::FlushActions()
{
    if (moved_ && !editNodes_.isEmpty() && editNodes_.size() == oldTransforms_.size())
    {
        ActionGroup group;

        for (int i = 0; i < editNodes_.size(); ++i)
        {
            QSharedPointer<EditNodeTransformAction> action(new EditNodeTransformAction(document_, *editNodes_[i], oldTransforms_[i]));
            group.actions_.push_back(action);
        }

        document_.AddAction(group);
    }

    moved_ = false;
}

void Gizmo::UseGizmoKeyboard(SceneInputInterface& input, float timeStep)
{
    using namespace Urho3D;

    Configuration& config = document_.GetConfig();
    const HotKeyMode hotKeyMode = (HotKeyMode)config.GetValue(SceneEditor::VarHotKeyMode).toInt();
    const GizmoType editMode = (GizmoType)config.GetValue(SceneEditor::VarGizmoType).toInt();
    const float moveStep = config.GetValue(SceneEditor::VarSnapPositionStep).toFloat();
    const bool moveSnap = config.GetValue(SceneEditor::VarSnapPosition).toBool();
    const float rotateStep = config.GetValue(SceneEditor::VarSnapRotationStep).toFloat();
    const bool rotateSnap = config.GetValue(SceneEditor::VarSnapRotation).toBool();
    const float scaleStep = config.GetValue(SceneEditor::VarSnapScaleStep).toFloat();
    const bool scaleSnap = config.GetValue(SceneEditor::VarSnapScale).toBool();

    const SceneDocument::NodeSet editNodes = document_.GetSelectedNodesAndComponents();
    if (editNodes.empty() || editMode == GizmoType::Select)
        return;

    // Process continuous movement
    if (!input.IsKeyDown(Qt::Key_Control))
        return;

    Vector3 adjust(0, 0, 0);
    if (input.IsKeyDown(Qt::Key_Up))
        adjust.z_ = 1;
    if (input.IsKeyDown(Qt::Key_Down))
        adjust.z_ = -1;
    if (input.IsKeyDown(Qt::Key_Left))
        adjust.x_ = -1;
    if (input.IsKeyDown(Qt::Key_Right))
        adjust.x_ = 1;
    if (input.IsKeyDown(Qt::Key_PageUp))
        adjust.y_ = 1;
    if (input.IsKeyDown(Qt::Key_PageDown))
        adjust.y_ = -1;
    if (editMode == GizmoType::Scale)
    {
        if (input.IsKeyDown(Qt::Key_Plus))
            adjust = Vector3(1, 1, 1);
        if (input.IsKeyDown(Qt::Key_Minus))
            adjust = Vector3(-1, -1, -1);
    }

    if (adjust != Vector3::ZERO)
    {
        bool moved = false;
        adjust *= timeStep * 10;

        switch (editMode)
        {
        case GizmoType::Position:
            if (!moveSnap)
                moved = MoveNodes(adjust * moveStep);
            break;

        case GizmoType::Rotation:
            if (!rotateSnap)
                moved = RotateNodes(adjust * rotateStep);
            break;

        case GizmoType::Scale:
            if (!scaleSnap)
                moved = ScaleNodes(adjust * scaleStep);
            break;
        }

        if (moved)
            MarkMoved();
    }

    // Process snapped movement
    adjust = Vector3::ZERO;
    if (input.IsKeyPressed(Qt::Key_Up))
        adjust.z_ = 1;
    if (input.IsKeyPressed(Qt::Key_Down))
        adjust.z_ = -1;
    if (input.IsKeyPressed(Qt::Key_Left))
        adjust.x_ = -1;
    if (input.IsKeyPressed(Qt::Key_Right))
        adjust.x_ = 1;
    if (input.IsKeyPressed(Qt::Key_PageUp))
        adjust.y_ = 1;
    if (input.IsKeyPressed(Qt::Key_PageDown))
        adjust.y_ = -1;
    if (editMode == GizmoType::Scale)
    {
        if (input.IsKeyPressed(Qt::Key_Plus))
            adjust = Vector3(1, 1, 1);
        if (input.IsKeyPressed(Qt::Key_Minus))
            adjust = Vector3(-1, -1, -1);
    }

    if (adjust != Vector3::ZERO)
    {
        bool moved = false;

        switch (editMode)
        {
        case GizmoType::Position:
            if (moveSnap)
                moved = MoveNodes(adjust);
            break;

        case GizmoType::Rotation:
            if (rotateSnap)
                moved = RotateNodes(adjust * rotateStep);
            break;

        case GizmoType::Scale:
            if (scaleSnap)
                moved = ScaleNodes(adjust * scaleStep);
            break;
        }

        if (moved)
            MarkMoved();
    }
}

void Gizmo::UseGizmoMouse(const Urho3D::Ray& mouseRay)
{
    using namespace Urho3D;

    Configuration& config = document_.GetConfig();
    const GizmoType editMode = (GizmoType)config.GetValue(SceneEditor::VarGizmoType).toInt();

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
        {
            editNodes_ = document_.GetSelectedNodesAndComponents().toList();
            oldTransforms_.resize(editNodes_.size());
            for (int i = 0; i < editNodes_.size(); ++i)
                oldTransforms_[i].Define(*editNodes_[i]);
        }

        bool moved = false;

        if (editMode == GizmoType::Position)
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
        else if (editMode == GizmoType::Rotation)
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
        else if (editMode == GizmoType::Scale)
        {
            Vector3 adjust(0, 0, 0);
            if (axisX_.selected)
                adjust += Vector3(1, 0, 0) * (axisX_.t - axisX_.lastT);
            if (axisY_.selected)
                adjust += Vector3(0, 1, 0) * (axisY_.t - axisY_.lastT);
            if (axisZ_.selected)
                adjust += Vector3(0, 0, 1) * (axisZ_.t - axisZ_.lastT);

            // Special handling for uniform scale: use the unmodified X-axis movement only
            if (editMode == GizmoType::Scale && axisX_.selected && axisY_
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
}

bool Gizmo::MoveNodes(Urho3D::Vector3 adjust)
{
    using namespace Urho3D;

    Configuration& config = document_.GetConfig();
    const GizmoAxisMode axisMode = (GizmoAxisMode)config.GetValue(SceneEditor::VarGizmoAxisMode).toInt();
    const float snapScale = config.GetValue(SceneEditor::VarSnapFactor).toFloat();
    const float moveStep = config.GetValue(SceneEditor::VarSnapPositionStep).toFloat();
    const bool moveSnap = config.GetValue(SceneEditor::VarSnapPosition).toBool();

    bool moved = false;

    if (adjust.Length() > M_EPSILON)
    {
        for (int i = 0; i < editNodes_.size(); ++i)
        {
            if (moveSnap)
            {
                float moveStepScaled = moveStep * snapScale;
                adjust.x_ = Floor(adjust.x_ / moveStepScaled + 0.5) * moveStepScaled;
                adjust.y_ = Floor(adjust.y_ / moveStepScaled + 0.5) * moveStepScaled;
                adjust.z_ = Floor(adjust.z_ / moveStepScaled + 0.5) * moveStepScaled;
            }

            Node& node = *editNodes_[i];
            Vector3 nodeAdjust = adjust;
            if (axisMode == GizmoAxisMode::Local && editNodes_.size() == 1)
                nodeAdjust = node.GetWorldRotation() * nodeAdjust;

            Vector3 worldPos = node.GetWorldPosition();
            Vector3 oldPos = node.GetPosition();

            worldPos += nodeAdjust;

            if (!node.GetParent())
                node.SetPosition(worldPos);
            else
                node.SetPosition(node.GetParent()->WorldToLocal(worldPos));

            if (node.GetPosition() != oldPos)
                moved = true;
        }
    }

    return moved;
}

bool Gizmo::RotateNodes(Urho3D::Vector3 adjust)
{
    using namespace Urho3D;

    Configuration& config = document_.GetConfig();
    const GizmoAxisMode axisMode = (GizmoAxisMode)config.GetValue(SceneEditor::VarGizmoAxisMode).toInt();
    const float snapScale = config.GetValue(SceneEditor::VarSnapFactor).toFloat();
    const float rotateStep = config.GetValue(SceneEditor::VarSnapRotationStep).toFloat();
    const bool rotateSnap = config.GetValue(SceneEditor::VarSnapRotation).toBool();

    bool moved = false;

    if (rotateSnap)
    {
        float rotateStepScaled = rotateStep * snapScale;
        adjust.x_ = Floor(adjust.x_ / rotateStepScaled + 0.5) * rotateStepScaled;
        adjust.y_ = Floor(adjust.y_ / rotateStepScaled + 0.5) * rotateStepScaled;
        adjust.z_ = Floor(adjust.z_ / rotateStepScaled + 0.5) * rotateStepScaled;
    }

    if (adjust.Length() > M_EPSILON)
    {
        moved = true;

        for (int i = 0; i < editNodes_.size(); ++i)
        {
            Node& node = *editNodes_[i];
            Quaternion rotQuat(adjust.x_, adjust.y_, adjust.z_);
            if (axisMode == GizmoAxisMode::Local && editNodes_.size() == 1)
                node.SetRotation(node.GetRotation() * rotQuat);
            else
            {
                Vector3 offset = node.GetWorldPosition() - axisX_.axisRay.origin_;
                if (node.GetParent() && node.GetParent()->GetWorldRotation() != Quaternion(1, 0, 0, 0))
                    rotQuat = node.GetParent()->GetWorldRotation().Inverse() * rotQuat * node.GetParent()->GetWorldRotation();
                node.SetRotation(rotQuat * node.GetRotation());
                Vector3 newPosition = axisX_.axisRay.origin_ + rotQuat * offset;
                if (node.GetParent())
                    newPosition = node.GetParent()->WorldToLocal(newPosition);
                node.SetPosition(newPosition);
            }
        }
    }

    return moved;
}

bool Gizmo::ScaleNodes(Urho3D::Vector3 adjust)
{
    using namespace Urho3D;

    Configuration& config = document_.GetConfig();
    const float snapScale = config.GetValue(SceneEditor::VarSnapFactor).toFloat();
    const float scaleStep = config.GetValue(SceneEditor::VarSnapScaleStep).toFloat();
    const bool scaleSnap = config.GetValue(SceneEditor::VarSnapScale).toBool();

    bool moved = false;

    if (adjust.Length() > M_EPSILON)
    {
        for (int i = 0; i < editNodes_.size(); ++i)
        {
            Node& node = *editNodes_[i];

            Vector3 scale = node.GetScale();
            Vector3 oldScale = scale;

            if (!scaleSnap)
                scale += adjust;
            else
            {
                float scaleStepScaled = scaleStep * snapScale;
                if (adjust.x_ != 0)
                {
                    scale.x_ += adjust.x_ * scaleStepScaled;
                    scale.x_ = Floor(scale.x_ / scaleStepScaled + 0.5) * scaleStepScaled;
                }
                if (adjust.y_ != 0)
                {
                    scale.y_ += adjust.y_ * scaleStepScaled;
                    scale.y_ = Floor(scale.y_ / scaleStepScaled + 0.5) * scaleStepScaled;
                }
                if (adjust.z_ != 0)
                {
                    scale.z_ += adjust.z_ * scaleStepScaled;
                    scale.z_ = Floor(scale.z_ / scaleStepScaled + 0.5) * scaleStepScaled;
                }
            }

            if (scale != oldScale)
                moved = true;

            node.SetScale(scale);
        }
    }

    return moved;
}

}
