#include "DebugGeometryRenderer.h"
#include "Selection.h"
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Navigation/CrowdManager.h>
#include <Urho3D/Navigation/DynamicNavigationMesh.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

void DebugGeometryRenderer::SetScene(Scene* scene)
{
    scene_ = scene;
}

void DebugGeometryRenderer::SetSelection(Selection* selection)
{
    selection_ = selection;
}

void DebugGeometryRenderer::DisableForComponent(const String& component)
{
    disabledForComponents_.Insert(component);
}

void DebugGeometryRenderer::PostRenderUpdate(AbstractInput& input, AbstractEditorContext& editorContext)
{
    if (!scene_)
        return;

    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();
    if (enabled_ && debug)
    {
        DrawDebugGeometry(debug);
        DrawDebugComponents(debug);
    }
}

bool DebugGeometryRenderer::ShallDrawNodeDebug(Node* node)
{
    // Exception for the scene to avoid bringing the editor to its knees: drawing either the whole hierarchy or the subsystem-
    // components can have a large performance hit. Also skip nodes with some components.
    if (node == scene_)
        return false;

    for (Component* component : node->GetComponents())
        if (disabledForComponents_.Contains(component->GetTypeName()))
            return false;

    return true;
}

void DebugGeometryRenderer::DrawNodeDebug(Node* node, DebugRenderer* debug, bool drawNode /*= true*/)
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

void DebugGeometryRenderer::DrawDebugGeometry(DebugRenderer* debug)
{
    // Draw hovered object.
    if (Node* node = selection_->GetHoveredNode())
        DrawNodeDebug(node, debug);
    if (Component* component = selection_->GetHoveredComponent())
        DrawNodeDebug(component->GetNode(), debug);

    // Draw selected nodes
    for (Node* node : selection_->GetNodes())
        DrawNodeDebug(node, debug);

    // Draw selected components
    for (Component* component : selection_->GetComponents())
        component->DrawDebugGeometry(debug, false);

    // Draw Renderer
    if (debugRenderer_)
    {
        Renderer* renderer = scene_->GetSubsystem<Renderer>();
        renderer->DrawDebugGeometry(false);
    }
}

void DebugGeometryRenderer::DrawDebugComponents(DebugRenderer* debug)
{
    using namespace Urho3D;

    PhysicsWorld* physicsWorld = scene_->GetComponent<PhysicsWorld>();
    Octree* octree = scene_->GetComponent<Octree>();
    CrowdManager* crowdManager = scene_->GetComponent<CrowdManager>();

    if (debugPhysics_ && physicsWorld)
        physicsWorld->DrawDebugGeometry(true);

    if (debugOctree_ && octree)
        octree->DrawDebugGeometry(true);

    if (debugNavigation_ && crowdManager)
    {
        crowdManager->DrawDebugGeometry(true);

        PODVector<NavigationMesh*> navMeshes;
        scene_->GetComponents(navMeshes, true);
        for (NavigationMesh* navMesh : navMeshes)
            navMesh->DrawDebugGeometry(true);

        PODVector<DynamicNavigationMesh*> dynNavMeshes;
        scene_->GetComponents(dynNavMeshes, true);
        for (DynamicNavigationMesh* dynNavMesh : dynNavMeshes)
            dynNavMesh->DrawDebugGeometry(true);
    }
}


}
