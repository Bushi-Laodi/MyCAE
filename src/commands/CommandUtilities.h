#pragma once

#include "commands/WorkflowCommandContext.h"

#include <QStringList>

class GeometryWorkflowController;
class ProjectWorkflowController;
class SolverWorkflowController;

void writeLogMessages(LogPanel *logPanel, const QStringList &messages);
ProjectWorkflowController makeProjectWorkflow(const WorkflowCommandContext &context);
GeometryWorkflowController makeGeometryWorkflow(
    const WorkflowCommandContext &context,
    ProjectWorkflowController &projectWorkflow
);
SolverWorkflowController makeSolverWorkflow(
    const WorkflowCommandContext &context,
    ProjectWorkflowController &projectWorkflow
);
