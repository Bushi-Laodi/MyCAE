#pragma once

#include "commands/AppCommand.h"
#include "commands/WorkflowCommandContext.h"

#include <memory>

enum class FaceGroupEditCommandType
{
    CreateFromPick,
    AddPickedFaces,
    RemovePickedFaces,
    ClearFaces,
    Rename,
    Delete,
    SetLocalMeshSize,
    TogglePhysicalGroup
};

std::unique_ptr<AppCommand> makeFaceGroupEditCommand(
    WorkflowCommandContext context,
    FaceGroupEditCommandType type
);
