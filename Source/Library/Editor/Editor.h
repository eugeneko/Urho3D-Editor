#pragma once

#include "EditorInterfaces.h"
#include <Urho3D/Math/Ray.h>

namespace Urho3D
{

class AbstractInput;
class EditorViewportLayout;
class AbstractMainWindow;

/// Editor base.
class Editor : public Object
{
    URHO3D_OBJECT(Editor, Object);

public:
    /// Construct.
    Editor(AbstractMainWindow& mainWindow);
    /// Set editor context.
    void SetEditorContext(AbstractEditorContext* editorContext) { editorContext_ = editorContext; }
    /// Add overlay.
    /// \todo add priority
    void AddOverlay(AbstractEditorOverlay* overlay);
    /// Add editor subsystem.
    void AddSubsystem(Object* subsystem);

private:
    /// Handle update.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle post-render update.
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);

private:
    SharedPtr<AbstractEditorContext> editorContext_ = nullptr;
    AbstractInput* input_ = nullptr;
    Vector<SharedPtr<AbstractEditorOverlay>> overlays_;
    Vector<SharedPtr<Object>> subsystems_;
};

/// Standard Editor context.
class StandardEditorContext : public AbstractEditorContext
{
    URHO3D_OBJECT(StandardEditorContext, AbstractEditorContext);

public:
    /// Construct.
    StandardEditorContext(Context* context, EditorViewportLayout* viewportLayout);

    /// \see AbstractEditorContext::GetMouseRay
    Ray GetMouseRay() const override;
    /// \see AbstractEditorContext::GetCurrentCamera
    Camera* GetCurrentCamera() const override;

private:
    /// Viewport layout.
    EditorViewportLayout* viewportLayout_ = nullptr;
};

}
