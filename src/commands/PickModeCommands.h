#pragma once

#include "commands/AppCommand.h"
#include "commands/WorkflowCommandContext.h"

#include <memory>

std::unique_ptr<AppCommand> makePickModeCommand(WorkflowCommandContext context);
std::unique_ptr<AppCommand> makeClearPickCommand(WorkflowCommandContext context);
