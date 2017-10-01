#pragma once

#include "Editor.h"

namespace Urho3D
{

class Selection;
class Component;
class Scene;
class CompositeKeyBinding;

/// Object pick mode.
enum class ObjectSelectionMode
{
    /// Pick geometries.
    Geometries,
    /// Pick lights.
    Lights,
    /// Pick zones.
    Zones,
    /// Pick rigid bodies.
    Rigidbodies,
    /// Pick UI.
    UI
};

/// Performs raycast and object picking.
class ObjectSelector : public AbstractEditorOverlay
{
    URHO3D_OBJECT(ObjectSelector, AbstractEditorOverlay);

public:
    /// Object selector controls.
    enum Control
    {
        SELECT_COMPONENT,
        TOGGLE_COMPONENT,
        SELECT_NODE,
        TOGGLE_NODE
    };
    using Controls = HashMap<int, CompositeKeyBinding>;
    /// Construct.
    ObjectSelector(Context* context) : AbstractEditorOverlay(context) { }
    /// Set scene.
    void SetScene(Scene* scene);
    /// Set selection.
    void SetSelection(Selection* selection);
    /// Set controls.
    void SetControls(const Controls& controls);
    /// Set selection mode.
    void SetSelectionMode(ObjectSelectionMode selectionMode) { selectionMode_ = selectionMode; }
    /// Add selection transferring routine.
    void AddSelectionTransferring(const String& sourceType, const String& destType);

private:
    /// \see AbstractEditorOverlay::PostRenderUpdate
    void PostRenderUpdate(AbstractEditorInput& input) override;

private:
    /// Perform raycast.
    void PerformRaycast(AbstractEditorInput& input);
    /// Resolve routine.
    Component* ResolveRoutine(Component* source);

private:
    /// Scene.
    SharedPtr<Scene> scene_;
    /// Selection.
    SharedPtr<Selection> selection_;
    /// Controls.
    Controls controls_;
    /// Object selection mode.
    ObjectSelectionMode selectionMode_ = ObjectSelectionMode::Geometries;
    /// Disabled components.
    HashMap<String, String> selectionTransferringRoutines_;

};

}
