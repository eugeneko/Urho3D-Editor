#include "ObjectPicker.h"
#include "SceneDocument.h"
#include "SceneEditor.h"
#include "../Configuration.h"
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3DEditor
{

ObjectPicker::ObjectPicker(SceneDocument& document)
    : document_(document)
{
    document_.AddOverlay(this);
}

void ObjectPicker::PostRenderUpdate(SceneInputInterface& input)
{
    PerformRaycast(input);
}

void ObjectPicker::PerformRaycast(SceneInputInterface& input)
{
    Configuration& config = document_.GetConfig();
    const HotKeyMode hotKeyMode = (HotKeyMode)config.GetValue(SceneEditor::VarHotKeyMode).toInt();
    const ObjectPickMode pickMode = ObjectPickMode::Geometries;

    using namespace Urho3D;
    Scene& scene = document_.GetScene();
    Camera& camera = document_.GetCurrentCamera();
    DebugRenderer* debug = scene.GetComponent<DebugRenderer>();

    const Ray cameraRay = input.GetMouseRay();
    Component* selectedComponent = nullptr;

    if (!input.IsMouseMoveConsumed())
    {
        if (pickMode == ObjectPickMode::Rigidbodies)
        {
            PhysicsWorld* physicsWorld = scene.GetComponent<PhysicsWorld>();
            if (!physicsWorld)
                return;

            // If we are not running the actual physics update, refresh collisions before raycasting
            //if (!runUpdate) #TODO Fixme
            physicsWorld->UpdateCollisions();

            PhysicsRaycastResult result;
            physicsWorld->RaycastSingle(result, cameraRay, camera.GetFarClip());
            if (result.body_)
            {
                RigidBody* body = result.body_;
                if (debug)
                {
                    debug->AddNode(body->GetNode(), 1.0, false);
                    body->DrawDebugGeometry(debug, false);
                }
                selectedComponent = body;
            }
        }
        else
        {
            Octree* octree = scene.GetComponent<Octree>();
            if (!octree)
                return;

            static int pickModeDrawableFlags[3] = { DRAWABLE_GEOMETRY, DRAWABLE_LIGHT, DRAWABLE_ZONE };
            PODVector<RayQueryResult> result;
            RayOctreeQuery query(result, cameraRay, RAY_TRIANGLE, camera.GetFarClip(), pickModeDrawableFlags[(int)pickMode], 0x7fffffff);
            octree->RaycastSingle(query);

            if (!result.Empty())
            {
                Drawable* drawable = result[0].drawable_;

                // If selecting a terrain patch, select the parent terrain instead
                if (drawable->GetTypeName() != "TerrainPatch")
                {
                    selectedComponent = drawable;
                    if (debug)
                    {
                        debug->AddNode(drawable->GetNode(), 1.0, false);
                        drawable->DrawDebugGeometry(debug, false);
                    }
                }
                else if (drawable->GetNode()->GetParent())
                    selectedComponent = drawable->GetNode()->GetParent()->GetComponent("Terrain");
            }
        }
    }

    bool multiselect = false;
    bool componentSelectQualifier = false;
    bool mouseButtonPressRL = false;

    if (hotKeyMode == HotKeyMode::Standard)
    {
        mouseButtonPressRL = input.IsMouseButtonPressed(Qt::LeftButton) && input.TryConsumeMouseButton(Qt::LeftButton);
        componentSelectQualifier = input.IsKeyDown(Qt::Key_Shift);
        multiselect = input.IsKeyDown(Qt::Key_Control);
    }
    else if (hotKeyMode == HotKeyMode::Blender)
    {
        mouseButtonPressRL = input.IsMouseButtonPressed(Qt::RightButton) && input.TryConsumeMouseButton(Qt::RightButton);
        componentSelectQualifier = input.IsKeyDown(Qt::Key_Control);
        multiselect = input.IsKeyDown(Qt::Key_Shift);
    }

    if (mouseButtonPressRL)
    {
        if (selectedComponent)
        {
            if (componentSelectQualifier)
            {
                // If we are selecting components, but have nodes in existing selection, do not multiselect to prevent confusion
                if (!document_.GetSelectedNodes().empty())
                    multiselect = false;
                document_.SelectObject(selectedComponent,
                    multiselect ? SelectionAction::Flip : SelectionAction::Select, !multiselect);
            }
            else
            {
                // If we are selecting nodes, but have components in existing selection, do not multiselect to prevent confusion
                if (!document_.GetSelectedComponents().empty())
                    multiselect = false;
                document_.SelectObject(selectedComponent->GetNode(),
                    multiselect ? SelectionAction::Flip : SelectionAction::Select, !multiselect);
            }
        }
        else
        {
            // If clicked on emptiness in non-multiselect mode, clear the selection
            if (!multiselect)
                document_.ClearSelection();
        }
    }
}

}

