#pragma once

#include <Urho3D/Core/Object.h>

namespace Urho3D
{

class UndoCommand : public Object
{
    URHO3D_OBJECT(UndoCommand, Object);

public:
    UndoCommand(Context* context) : Object(context) { }
    virtual void Undo() const = 0;
    virtual void Redo() const = 0;
    virtual String GetTitle() { return String::EMPTY; }

};

using UndoCommandSPtr = SharedPtr<UndoCommand>;

class UndoCommandGroup : public UndoCommand
{
    URHO3D_OBJECT(UndoCommandGroup, UndoCommand);

public:
    UndoCommandGroup(Context* context, const String& title) : UndoCommand(context), title_(title) { }
    void Undo() const override
    {
        for (int i = static_cast<int>(commands_.Size()) - 1; i >= 0; --i)
            commands_[i]->Undo();
    }
    void Redo() const override
    {
        for (UndoCommand* command : commands_)
            command->Redo();
    }
    String GetTitle() override { return title_; }

    void Push(const UndoCommandSPtr& command) { commands_.Push(command); }

private:
    const String title_;
    Vector<SharedPtr<UndoCommand>> commands_;
};

class UndoStack : public Object
{
    URHO3D_OBJECT(UndoStack, Object);

public:
    UndoStack(Context* context) : Object(context) { }
    void SetLimit(unsigned limit) { limit_ = limit; }
    void Push(const UndoCommandSPtr& command, bool redo = true);
    bool Undo();
    bool Redo();
    bool CanUndo() const;
    bool CanRedo() const;
    String GetUndoTitle() const;
    String GetRedoTitle() const;

private:
    unsigned limit_ = 50;
    Vector<UndoCommandSPtr> undoStack_;
    Vector<UndoCommandSPtr> redoStack_;
};

}
