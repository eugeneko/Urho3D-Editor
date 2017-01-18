#pragma once

#include "SceneOverlay.h"
#include <QObject>

namespace Urho3DEditor
{

class SceneDocument;

/// Object pick mode.
enum class ObjectPickMode
{
    /// Pick geometries.
    Geometries,
    /// Pick lights.
    Lights,
    /// Pick zones.
    Zones,
    /// Pick rigid bodies.
    Rigidbodies
};

/// Performs raycast and object picking.
class ObjectPicker : public QObject, public SceneOverlay
{
    Q_OBJECT

public:
    /// Construct.
    ObjectPicker(SceneDocument& document);

private:
    /// @see SceneOverlay::PostRenderUpdate
    virtual void PostRenderUpdate(SceneInputInterface& input) override;

private:
    /// Perform raycast.
    void PerformRaycast(SceneInputInterface& input);

private:
    /// Document.
    SceneDocument& document_;

};

}
