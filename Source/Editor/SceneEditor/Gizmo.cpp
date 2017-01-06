#include "Gizmo.h"
#include "SceneEditor.h"
#include "../Configuration.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Resource/ResourceCache.h>

namespace Urho3DEditor
{

const QString GizmoManager::VarGizmoMode = "scene/gizmo/mode";

bool GizmoManager::Initialize()
{
    Configuration& config = GetConfig();
    QStringList gizmoTypeNames;
    gizmoTypeNames << "None" << "Position" << "Rotation" << "Scale";
    config.RegisterVariable(VarGizmoMode, Gizmo::GizmoNone, "Scene.Gizmo/Mode", gizmoTypeNames);
    return true;
}

//////////////////////////////////////////////////////////////////////////
Gizmo::Gizmo(SceneDocument& page)
    : page_(page)
    , gizmoNode_(page.GetContext())
    , gizmo_(*gizmoNode_.CreateComponent<Urho3D::StaticModel>())
    , type_(GizmoNone)
{
    // Setup gizmo
    // #TODO Make configurable
    Urho3D::ResourceCache* cache = page.GetContext()->GetSubsystem<Urho3D::ResourceCache>();
    gizmo_.SetModel(cache->GetResource<Urho3D::Model>("Models/Editor/Axes.mdl"));
    gizmo_.SetMaterial(0, cache->GetResource<Urho3D::Material>("Materials/Editor/RedUnlit.xml"));
    gizmo_.SetMaterial(1, cache->GetResource<Urho3D::Material>("Materials/Editor/GreenUnlit.xml"));
    gizmo_.SetMaterial(2, cache->GetResource<Urho3D::Material>("Materials/Editor/BlueUnlit.xml"));
    gizmo_.SetEnabled(false);
    gizmo_.SetViewMask(0x80000000);
    gizmo_.SetOccludee(false);
    gizmoNode_.SetName("EditorGizmo");

    connect(&page_, SIGNAL(selectionChanged()), this, SLOT(HandleSelectionChanged()));

    // #TODO Use me
    Configuration& config = page_.GetConfig();

    SetType(GizmoPosition);
}

Gizmo::~Gizmo()
{
}

void Gizmo::SetType(GizmoType type)
{
    if (type_ != type)
    {
        type_ = type;
        CreateGizmo();
    }
}

void Gizmo::HandleSelectionChanged()
{
    CreateGizmo();
}

void Gizmo::HideGizmo()
{
    gizmo_.SetEnabled(false);
}

void Gizmo::CreateGizmo()
{
    if (type_ == GizmoNone)
    {
        HideGizmo();
        return;
    }

    using namespace Urho3D;

    // Gather nodes
    SceneDocument::NodeSet editNodes = page_.GetSelectedNodes();
    const SceneDocument::ComponentSet selectedComponents = page_.GetSelectedComponents();
    for (Component* component : selectedComponents)
        editNodes.insert(component->GetNode());

    // Hide gizmo if scene included
    if (editNodes.contains(&page_.GetScene()))
    {
        HideGizmo();
        return;
    }

    // Compute gizmo position
    Vector3 gizmoPosition;
    for (Node* node : editNodes)
        gizmoPosition += node->GetWorldPosition();
    gizmoPosition /= editNodes.size();

    // Setup gizmo
    gizmoNode_.SetWorldPosition(gizmoPosition);
    gizmoNode_.SetWorldRotation(Quaternion());
    if (Octree* octree = page_.GetScene().GetComponent<Octree>())
        octree->AddManualDrawable(&gizmo_);
}

}
