#pragma once

#include "commands/AppCommand.h"
#include "commands/WorkflowCommandContext.h"

#include <memory>

std::unique_ptr<AppCommand> makeToggleGeometryEdgesCommand(WorkflowCommandContext context);
std::unique_ptr<AppCommand> makeToggleMeshTransparencyCommand(WorkflowCommandContext context);
std::unique_ptr<AppCommand> makeToggleOrientationMarkerCommand(WorkflowCommandContext context);
