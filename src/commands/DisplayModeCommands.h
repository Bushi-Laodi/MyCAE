#pragma once

#include "commands/AppCommand.h"
#include "commands/WorkflowCommandContext.h"

#include <memory>

std::unique_ptr<AppCommand> makeToggleGeometryEdgesCommand(WorkflowCommandContext context);
