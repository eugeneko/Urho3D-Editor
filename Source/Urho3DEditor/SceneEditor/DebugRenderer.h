#pragma once

#include "SceneOverlay.h"
#include <QObject>

namespace Urho3D
{

class Node;
class DebugRenderer;

}

namespace Urho3DEditor
{

class Configuration;
class SceneDocument;

class DebugRenderer : public QObject, public SceneOverlay
{
    Q_OBJECT

public:
    /// Controls whether debug renderer is disabled.
    static const QString VarDisable;
    /// Controls components excluded from debug renderer.
    static const QString VarDisableDebugForComponents;
    /// Controls whether the debug renderer is enabled.
    static const QString VarDebugRenderer;
    /// Controls whether the physics debug is enabled.
    static const QString VarDebugPhysics;
    /// Controls whether the octree debug is enabled.
    static const QString VarDebugOctree;
    /// Controls whether the navigation debug is enabled.
    static const QString VarDebugNavigation;

    /// Register variables.
    static void RegisterVariables(Configuration& config);

public:
    /// Construct.
    DebugRenderer(SceneDocument& document);

private:
    /// @see DebugRenderer::PostRenderUpdate
    virtual void PostRenderUpdate(SceneInputInterface& input) override;

private:
    /// Check whether to draw debug geometry for node.
    bool ShallDrawNodeDebug(Urho3D::Node* node);
    /// Draw node debug geometry.
    void DrawNodeDebug(Urho3D::Node* node, Urho3D::DebugRenderer* debug, bool drawNode = true);
    /// Draw debug geometry.
    void DrawDebugGeometry();
    /// Draw debug components.
    void DrawDebugComponents();

private:
    /// Document.
    SceneDocument& document_;

};

}
