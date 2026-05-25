#pragma once

#include "commands/AppCommand.h"
#include "commands/WorkflowCommandContext.h"

#include <memory>

class QMainWindow;

enum class ProjectCommandType
{
    Create,
    Open
};

std::unique_ptr<AppCommand> makeProjectCommand(WorkflowCommandContext context, ProjectCommandType type);
std::unique_ptr<AppCommand> makeCloseWindowCommand(QMainWindow *window);
