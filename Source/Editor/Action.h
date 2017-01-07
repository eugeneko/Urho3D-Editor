#pragma once

#include <QSharedPointer>
#include <QVector>

namespace Urho3DEditor
{

/// Concept of editor action.
class Action
{
    /// Undo action.
    virtual void Undo() = 0;
    /// Redo action.
    virtual void Redo() = 0;
};

/// Action group.
struct ActionGroup
{
    /// Actions.
    QVector<QSharedPointer<Action>> actions_;
};

}
