#include "Gizmo.h"
#include "SceneActions.h"
#include "SceneDocument.h"
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

const QString GizmoManager::VarGizmoType =     "scene.gizmo/type";
const QString GizmoManager::VarGizmoAxisMode = "scene.gizmo/axismode";

const QString GizmoManager::VarSnapFactor =       "scene.gizmo/snap.factor";
const QString GizmoManager::VarSnapPosition =     "scene.gizmo/snap.position";
const QString GizmoManager::VarSnapRotation =     "scene.gizmo/snap.rotation";
const QString GizmoManager::VarSnapScale =        "scene.gizmo/snap.scale";
const QString GizmoManager::VarSnapPositionStep = "scene.gizmo/step.position";
const QString GizmoManager::VarSnapRotationStep = "scene.gizmo/step.rotation";
const QString GizmoManager::VarSnapScaleStep =    "scene.gizmo/step.scale";

const QString GizmoManager::VarModelPosition = "scene.gizmo/model.position";
const QString GizmoManager::VarModelRotation = "scene.gizmo/model.rotation";
const QString GizmoManager::VarModelScale =    "scene.gizmo/model.scale";

const QString GizmoManager::VarMaterialRed =   "scene.gizmo/material.red";
const QString GizmoManager::VarMaterialGreen = "scene.gizmo/material.green";
const QString GizmoManager::VarMaterialBlue =  "scene.gizmo/material.blue";

const QString GizmoManager::VarMaterialRedHighlight =   "scene.gizmo/material.red.h";
const QString GizmoManager::VarMaterialGreenHighlight = "scene.gizmo/material.green.h";
const QString GizmoManager::VarMaterialBlueHighlight =  "scene.gizmo/material.blue.h";

bool GizmoManager::Initialize()
{
    MainWindow& mainWindow = GetMainWindow();
    connect(&mainWindow, SIGNAL(pageChanged(Document*)), this, SLOT(HandleCurrentPageChanged(Document*)));

    Configuration& config = GetConfig();

    QStringList gizmoTypeNames;
    gizmoTypeNames << "Position" << "Rotation" << "Scale" << "Select";
    config.RegisterVariable(VarGizmoType, (int)GizmoType::Position, "Scene.Gizmo", "Type", gizmoTypeNames);

    QStringList gizmoAxisModeNames;
    gizmoAxisModeNames << "Local" << "World";
    config.RegisterVariable(VarGizmoAxisMode, (int)GizmoAxisMode::Local, "Scene.Gizmo", "Axis Mode", gizmoAxisModeNames);

    config.RegisterVariable(VarSnapFactor,       1.0,   "Scene.Gizmo", "Snap Factor");
    config.RegisterVariable(VarSnapPosition,     false, "Scene.Gizmo", "Snap Position");
    config.RegisterVariable(VarSnapRotation,     false, "Scene.Gizmo", "Snap Rotation");
    config.RegisterVariable(VarSnapScale,        false, "Scene.Gizmo", "Snap Scale");
    config.RegisterVariable(VarSnapPositionStep, 0.5,   "Scene.Gizmo", "Position Step");
    config.RegisterVariable(VarSnapRotationStep, 5.0,   "Scene.Gizmo", "Rotation Step");
    config.RegisterVariable(VarSnapScaleStep,    1.0,   "Scene.Gizmo", "Scale Step");

    config.RegisterVariable(VarModelPosition, "Models/Editor/Axes.mdl",       "Scene.Gizmo", "Model Position");
    config.RegisterVariable(VarModelRotation, "Models/Editor/RotateAxes.mdl", "Scene.Gizmo", "Model Rotation");
    config.RegisterVariable(VarModelScale,    "Models/Editor/ScaleAxes.mdl",  "Scene.Gizmo", "Model Scale");

    config.RegisterVariable(VarMaterialRed,   "Materials/Editor/RedUnlit.xml",   "Scene.Gizmo", "Material Red");
    config.RegisterVariable(VarMaterialGreen, "Materials/Editor/GreenUnlit.xml", "Scene.Gizmo", "Material Green");
    config.RegisterVariable(VarMaterialBlue,  "Materials/Editor/BlueUnlit.xml",  "Scene.Gizmo", "Material Blue");

    config.RegisterVariable(VarMaterialRedHighlight,   "Materials/Editor/BrightRedUnlit.xml",   "Scene.Gizmo", "Material Red (Highlight)");
    config.RegisterVariable(VarMaterialGreenHighlight, "Materials/Editor/BrightGreenUnlit.xml", "Scene.Gizmo", "Material Green (Highlight)");
    config.RegisterVariable(VarMaterialBlueHighlight,  "Materials/Editor/BrightBlueUnlit.xml",  "Scene.Gizmo", "Material Blue (Highlight)");

    return true;
}

void GizmoManager::HandleCurrentPageChanged(Document* document)
{
    if (SceneDocument* sceneDocument = dynamic_cast<SceneDocument*>(document))
    {
        sceneDocument->Get<Gizmo, SceneDocument>();
    }
}

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
    , drag_(false)
    , lastDrag_(false)
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

void Gizmo::Update(SceneInputInterface& input, const Urho3D::Ray& cameraRay, float timeStep)
{
    drag_ = input.IsMouseButtonDown(Qt::LeftButton);
    UseGizmo(cameraRay);
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
        variableName = GizmoManager::VarModelPosition;
        break;
    case GizmoType::Rotation:
        variableName = GizmoManager::VarModelRotation;
        break;
    case GizmoType::Scale:
        variableName = GizmoManager::VarModelScale;
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
        variableName = highlight ? GizmoManager::VarMaterialRedHighlight : GizmoManager::VarMaterialRed;
        break;
    case 1:
        variableName = highlight ? GizmoManager::VarMaterialGreenHighlight : GizmoManager::VarMaterialGreen;
        break;
    case 2:
        variableName = highlight ? GizmoManager::VarMaterialBlueHighlight : GizmoManager::VarMaterialBlue;
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

void Gizmo::PositionGizmo()
{
    using namespace Urho3D;

    Configuration& config = document_.GetConfig();
    const GizmoType type = (GizmoType)config.GetValue(GizmoManager::VarGizmoType).toInt();
    const GizmoAxisMode axisMode = (GizmoAxisMode)config.GetValue(GizmoManager::VarGizmoAxisMode).toInt();

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

void Gizmo::UseGizmo(const Urho3D::Ray& cameraRay)
{
    using namespace Urho3D;

    Configuration& config = document_.GetConfig();
    const GizmoType type = (GizmoType)config.GetValue(GizmoManager::VarGizmoType).toInt();

    if (!gizmo_.IsEnabled() || type == GizmoType::Select)
    {
        FlushActions();
        lastDrag_ = false;
    }

    const float scale = gizmoNode_.GetScale().x_;

    // Recalculate axes only when not left-dragging
    if (!drag_)
        CalculateGizmoAxes();

    // #TODO Move to config
    const float axisMaxD = 0.1f;
    const float axisMaxT = 1.0f;
    const float rotSensitivity = 50.0f;

    axisX_.Update(cameraRay, scale, drag_, axisMaxT, axisMaxD);
    axisY_.Update(cameraRay, scale, drag_, axisMaxT, axisMaxD);
    axisZ_.Update(cameraRay, scale, drag_, axisMaxT, axisMaxD);

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

    if (drag_)
    {
        // Store initial transforms for undo when gizmo drag started
        if (!lastDrag_)
        {
            editNodes_ = document_.GetSelectedNodesAndComponents().toList();
            oldTransforms_.resize(editNodes_.size());
            for (int i = 0; i < editNodes_.size(); ++i)
                oldTransforms_[i].Define(*editNodes_[i]);
        }

        bool moved = false;

        if (type == GizmoType::Position)
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
        else if (type == GizmoType::Rotation)
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
        else if (type == GizmoType::Scale)
        {
            Vector3 adjust(0, 0, 0);
            if (axisX_.selected)
                adjust += Vector3(1, 0, 0) * (axisX_.t - axisX_.lastT);
            if (axisY_.selected)
                adjust += Vector3(0, 1, 0) * (axisY_.t - axisY_.lastT);
            if (axisZ_.selected)
                adjust += Vector3(0, 0, 1) * (axisZ_.t - axisZ_.lastT);

            // Special handling for uniform scale: use the unmodified X-axis movement only
            if (type == GizmoType::Scale && axisX_.selected && axisY_
                .selected && axisZ_.selected)
            {
                float x = axisX_.t - axisX_.lastT;
                adjust = Vector3(x, x, x);
            }

            moved = ScaleNodes(adjust);
        }

        if (moved)
        {
            MarkMoved();
            for (Node* node : editNodes_)
                emit document_.nodeTransformChanged(*node);
        }
    }
    else
    {
        if (lastDrag_)
            FlushActions();
    }

    lastDrag_ = drag_;
}

bool Gizmo::MoveNodes(Urho3D::Vector3 adjust)
{
    using namespace Urho3D;

    Configuration& config = document_.GetConfig();
    const GizmoAxisMode axisMode = (GizmoAxisMode)config.GetValue(GizmoManager::VarGizmoAxisMode).toInt();
    const float snapScale = config.GetValue(GizmoManager::VarSnapFactor).toFloat();
    const float moveStep = config.GetValue(GizmoManager::VarSnapPositionStep).toFloat();
    const bool moveSnap = config.GetValue(GizmoManager::VarSnapPosition).toBool();

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
    const GizmoAxisMode axisMode = (GizmoAxisMode)config.GetValue(GizmoManager::VarGizmoAxisMode).toInt();
    const float snapScale = config.GetValue(GizmoManager::VarSnapFactor).toFloat();
    const float rotateStep = config.GetValue(GizmoManager::VarSnapRotationStep).toFloat();
    const bool rotateSnap = config.GetValue(GizmoManager::VarSnapRotation).toBool();

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
    const float snapScale = config.GetValue(GizmoManager::VarSnapFactor).toFloat();
    const float scaleStep = config.GetValue(GizmoManager::VarSnapScaleStep).toFloat();
    const bool scaleSnap = config.GetValue(GizmoManager::VarSnapScale).toBool();

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
