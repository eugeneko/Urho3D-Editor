#include "UndoStack.h"

namespace Urho3D
{

void UndoStack::Push(const UndoCommandSPtr& command, bool redo /*= true*/)
{
    if (undoStack_.Size() >= limit_)
        undoStack_.Erase(0, undoStack_.Size() - limit_ + 1);

    undoStack_.Push(command);
    redoStack_.Clear();
    if (redo)
        command->Redo();
}

bool UndoStack::Undo()
{
    if (!CanUndo())
        return false;

    UndoCommandSPtr command = undoStack_.Back();
    undoStack_.Pop();
    command->Undo();
    redoStack_.Push(command);

    return true;
}

bool UndoStack::Redo()
{
    if (!CanRedo())
        return false;

    UndoCommandSPtr command = redoStack_.Back();
    redoStack_.Pop();
    command->Redo();
    undoStack_.Push(command);

    return true;
}

bool UndoStack::CanUndo() const
{
    return !undoStack_.Empty();
}

bool UndoStack::CanRedo() const
{
    return !redoStack_.Empty();
}

String UndoStack::GetUndoTitle() const
{
    return CanUndo() ? undoStack_.Back()->GetTitle() : String::EMPTY;
}

String UndoStack::GetRedoTitle() const
{
    return CanRedo() ? redoStack_.Back()->GetTitle() : String::EMPTY;
}

}
