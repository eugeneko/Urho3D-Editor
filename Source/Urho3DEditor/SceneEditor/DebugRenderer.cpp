#include "DebugRenderer.h"
#include "SceneDocument.h"
#include "../Core/QtUrhoHelpers.h"
#include "../Configuration.h"
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Navigation/CrowdManager.h>
#include <Urho3D/Navigation/DynamicNavigationMesh.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3DEditor
{

const QString DebugRenderer::VarDisable = "scene.debug/disable";
const QString DebugRenderer::VarDisableDebugForComponents = "scene.debug/disablecomponents";
const QString DebugRenderer::VarDebugRenderer = "scene.debug/renderer";
const QString DebugRenderer::VarDebugPhysics = "scene.debug/physics";
const QString DebugRenderer::VarDebugOctree = "scene.debug/octree";
const QString DebugRenderer::VarDebugNavigation = "scene.debug/navigation";

void DebugRenderer::RegisterVariables(Configuration& config)
{
    config.RegisterVariable(VarDisable, false, "Scene.Debug", "Disable rendering");
    config.RegisterVariable(VarDisableDebugForComponents, QStringList("Terrain"), "Scene.Debug", "Disable for components");
    config.RegisterVariable(VarDebugRenderer, false, "Scene.Debug", "Render debug geometry");
    config.RegisterVariable(VarDebugPhysics, false, "Scene.Debug", "Render physics debug");
    config.RegisterVariable(VarDebugOctree, false, "Scene.Debug", "Render octree debug");
    config.RegisterVariable(VarDebugNavigation, false, "Scene.Debug", "Render navigation debug");
}

DebugRenderer::DebugRenderer(SceneDocument& document)
    : document_(document)
{
    document_.AddOverlay(this);
}

void DebugRenderer::PostRenderUpdate(SceneInputInterface& input)
{
    Configuration& config = document_.GetConfig();
    const bool debugRendererDisabled = config.GetValue(VarDisable).toBool();

    Urho3D::Scene& scene = document_.GetScene();
    Urho3D::DebugRenderer* debug = scene.GetComponent<Urho3D::DebugRenderer>();
    if (debug && !debugRendererDisabled)
    {
        DrawDebugGeometry();
        DrawDebugComponents();
    }
}

bool DebugRenderer::ShallDrawNodeDebug(Urho3D::Node* node)
{
    // Exception for the scene to avoid bringing the editor to its knees: drawing either the whole hierarchy or the subsystem-
    // components can have a large performance hit. Also skip nodes with some components.
    if (node == &document_.GetScene())
        return false;

    Configuration& config = document_.GetConfig();
    const QStringList exceptionComponents =
        config.GetValue(VarDisableDebugForComponents).toStringList();
    for (const QString& componentName : exceptionComponents)
        if (node->GetComponent(Cast(componentName)))
            return false;

    return true;
}

void DebugRenderer::DrawNodeDebug(Urho3D::Node* node, Urho3D::DebugRenderer* debug, bool drawNode /*= true*/)
{
    using namespace Urho3D;

    if (drawNode)
        debug->AddNode(node, 1.0, false);

    if (!ShallDrawNodeDebug(node))
        return;

    // Draw components
    const Vector<SharedPtr<Component>>& components = node->GetComponents();
    for (Component* component : components)
        component->DrawDebugGeometry(debug, false);

    // To avoid cluttering the view, do not draw the node axes for child nodes
    const Vector<SharedPtr<Node>>& children = node->GetChildren();
    for (Node* child : children)
        DrawNodeDebug(child, debug, false);
}

void DebugRenderer::DrawDebugGeometry()
{
    Configuration& config = document_.GetConfig();
    const bool debugRendering = config.GetValue(VarDebugRenderer).toBool();

    Urho3D::Scene& scene = document_.GetScene();
    Urho3D::DebugRenderer* debug = scene.GetComponent<Urho3D::DebugRenderer>();
    Urho3D::Renderer* renderer = scene.GetSubsystem<Urho3D::Renderer>();

    // Visualize the currently selected nodes
    for (Urho3D::Node* node : document_.GetSelectedNodes())
        DrawNodeDebug(node, debug);

    // Visualize the currently selected components
    for (Urho3D::Component* component : document_.GetSelectedComponents())
        component->DrawDebugGeometry(debug, false);

    if (debugRendering)
        renderer->DrawDebugGeometry(false);
}

void DebugRenderer::DrawDebugComponents()
{
    using namespace Urho3D;

    Configuration& config = document_.GetConfig();
    const bool debugPhysics = config.GetValue(VarDebugPhysics).toBool();
    const bool debugOctree = config.GetValue(VarDebugOctree).toBool();
    const bool debugNavigation = config.GetValue(VarDebugNavigation).toBool();

    Scene& scene = document_.GetScene();
    PhysicsWorld* physicsWorld = scene.GetComponent<PhysicsWorld>();
    Octree* octree = scene.GetComponent<Octree>();
    CrowdManager* crowdManager = scene.GetComponent<CrowdManager>();

    if (debugPhysics && physicsWorld)
        physicsWorld->DrawDebugGeometry(true);

    if (debugOctree && octree)
        octree->DrawDebugGeometry(true);

    if (debugNavigation && crowdManager)
    {
        crowdManager->DrawDebugGeometry(true);

        PODVector<NavigationMesh*> navMeshes;
        scene.GetComponents(navMeshes, true);
        for (NavigationMesh* navMesh : navMeshes)
            navMesh->DrawDebugGeometry(true);

        PODVector<DynamicNavigationMesh*> dynNavMeshes;
        scene.GetComponents(dynNavMeshes, true);
        for (DynamicNavigationMesh* dynNavMesh : dynNavMeshes)
            dynNavMesh->DrawDebugGeometry(true);
    }
}


}
