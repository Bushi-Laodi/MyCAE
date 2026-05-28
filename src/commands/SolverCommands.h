#pragma once

#include "commands/AppCommand.h"
#include "commands/WorkflowCommandContext.h"

#include <QString>

#include <memory>

enum class SolverDataCommandType
{
    CreateMaterial,
    CreateStructuralMaterial,
    CreateFluidMaterial,
    CreateBoundaryCondition,
    CreateStructuralBoundaryCondition,
    CreateCfdBoundaryCondition,
    CreateLoad,
    CreateStructuralLoad,
    CreateCfdFieldValue,
    EditSelected,
    DeleteSelected
};

std::unique_ptr<AppCommand> makeSolverDataCommand(WorkflowCommandContext context, SolverDataCommandType type);
std::unique_ptr<AppCommand> makeRunSolverCommand(WorkflowCommandContext context, const QString &pluginId);
