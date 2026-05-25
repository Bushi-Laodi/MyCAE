#pragma once

#include "commands/AppCommand.h"
#include "commands/WorkflowCommandContext.h"

#include <memory>

enum class MeshCommandType
{
    CheckGmsh,
    Generate,
    ReadInfo,
    Show
};

std::unique_ptr<AppCommand> makeMeshCommand(WorkflowCommandContext context, MeshCommandType type);
