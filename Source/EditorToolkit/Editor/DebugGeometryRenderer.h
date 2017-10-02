#pragma once

#include "EditorInterfaces.h"

namespace Urho3D
{

class Node;
class Scene;
class DebugRenderer;
class Selection;

class DebugGeometryRenderer : public AbstractEditorOverlay
{
    URHO3D_OBJECT(DebugGeometryRenderer, AbstractEditorOverlay);

public:
    /// Construct.
    DebugGeometryRenderer(Context* context) : AbstractEditorOverlay(context) { }
    /// Set scene.
    void SetScene(Scene* scene);
    /// Set selection.
    void SetSelection(Selection* selection);
    /// Set enabled.
    void SetEnabled(bool enabled) { enabled_ = enabled; }
    /// Set whether to draw debug renderer geometry.
    void SetDebugRenderer(bool debugRenderer) { debugRenderer_ = debugRenderer; }
    /// Set whether to draw debug octree geometry.
    void SetDebugOctree(bool debugOctree) { debugOctree_ = debugOctree; }
    /// Set whether to draw debug physics geometry.
    void SetDebugPhysics(bool debugPhysics) { debugPhysics_ = debugPhysics; }
    /// Set whether to draw debug navigation geometry.
    void SetDebugNavigation(bool debugNavigation) { debugNavigation_ = debugNavigation; }
    /// Disable for component.
    void DisableForComponent(const String& component);

private:
    /// \see AbstractEditorOverlay::PostRenderUpdate
    void PostRenderUpdate(AbstractEditorInput& input) override;

private:
    /// Check whether to draw debug geometry for node.
    bool ShallDrawNodeDebug(Node* node);
    /// Draw node debug geometry.
    void DrawNodeDebug(Node* node, DebugRenderer* debug, bool drawNode = true);
    /// Draw debug geometry.
    void DrawDebugGeometry(DebugRenderer* debug);
    /// Draw debug components.
    void DrawDebugComponents(DebugRenderer* debug);

private:
    /// Scene.
    SharedPtr<Scene> scene_;
    /// Selection.
    SharedPtr<Selection> selection_;

    bool enabled_ = true;
    bool debugRenderer_ = false;
    bool debugOctree_ = false;
    bool debugPhysics_ = false;
    bool debugNavigation_ = false;
    HashSet<String> disabledForComponents_;

};

}
