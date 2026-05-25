#pragma once

#include "commands/AppCommand.h"
#include "commands/WorkflowCommandContext.h"
#include "geometry/GeometryCreationController.h"

#include <memory>

std::unique_ptr<AppCommand> makeGeometryCreateCommand(WorkflowCommandContext context, GeometryCreateType type);
