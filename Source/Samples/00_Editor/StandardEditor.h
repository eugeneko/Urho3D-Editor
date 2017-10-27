#pragma once

#include "../../Library/AbstractUI/AbstractUI.h"
#include "../../Library/AbstractUI/KeyBinding.h"
#include "../../Library/Editor/CameraController.h"
#include "../../Library/Editor/Editor.h"
#include "../../Library/Editor/Selection.h"
#include "../../Library/Editor/HierarchyWindow.h"
#include "../../Library/Editor/ObjectSelector.h"
#include "../../Library/Editor/EditorViewportLayout.h"
#include "../../Library/Editor/DebugGeometryRenderer.h"
#include "../../Library/Editor/Gizmo.h"
#include "../../Library/Editor/ResourceBrowser.h"
#include "../../Library/Editor/Inspector.h"
#include "../../Library/Editor/Transformable.h"

namespace Urho3D
{

class StandardDocument : public Object
{
    URHO3D_OBJECT(StandardDocument, Object);

public:
    StandardDocument(Context* context) : Object(context) { }

    String resourceKey_;
    SharedPtr<Resource> resource_;
    SharedPtr<Scene> scene_;
    SharedPtr<Selection> selection_;
    SharedPtr<SelectionTransform> selectionTransform_;
};

class StandardEditor : public Object
{
    URHO3D_OBJECT(StandardEditor, Object);

public:
    StandardEditor(AbstractMainWindow* mainWindow, bool blenderHotkeys);
    void SwitchToDocument(StandardDocument* document);

private:
    void InitializeResourceLayers();
    void SetupMenu();
    void SetupUrhoControls();
    void SetupBlenderControls();

    template <class T> StandardDocument* FindDocument(T condition)
    {
        for (Object* document : mainWindow_->GetDocuments())
            if (condition(document))
                return static_cast<StandardDocument*>(document);
        return nullptr;
    }
    StandardDocument* FindDocumentForResource(const String& resourceKey);

    SharedPtr<StandardDocument> CreateDocumentForResource(const ResourceFileDesc& resource);
    SharedPtr<StandardDocument> CreateSceneDocument(Scene* scene, const String& resourceKey = String::EMPTY);

    void UpdateInspector();
    SharedPtr<Inspectable> CreateNodesInspector(const Selection::NodeVector& nodes) const;
    SharedPtr<Inspectable> CreateComponentsInspector(const Selection::ComponentVector& components) const;

private:
    AbstractMainWindow* mainWindow_ = nullptr;
    StandardDocument* currentDocument_ = nullptr;

    SharedPtr<Editor> editor_;
    SharedPtr<EditorViewportLayout> viewportLayout_;
    SharedPtr<DebugGeometryRenderer> debugGeometryRenderer_;
    SharedPtr<CameraController> cameraController_;
    SharedPtr<ObjectSelector> objectSelector_;
    SharedPtr<Gizmo> gizmo_;

    SharedPtr<HierarchyWindow> hierarchyWindow_;
    SharedPtr<Inspector> inspector_;
    SharedPtr<ResourceBrowser> resourceBrowser_;

    // #TODO Hide me
    unsigned maxInspectorLabelLength_ = 200;
};

}
