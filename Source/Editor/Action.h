#pragma once

#include <QSharedPointer>
#include <QVector>

namespace Urho3DEditor
{

/// Concept of editor action.
class Action
{
public:
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
    /// Undo all.
    void Undo()
    {
        for (int i = actions_.size() - 1; i >= 0; --i)
            actions_[i]->Undo();
    }
    /// Redo all.
    void Redo()
    {
        for (int i = 0; i < actions_.size(); ++i)
            actions_[i]->Redo();
    }
};

}
