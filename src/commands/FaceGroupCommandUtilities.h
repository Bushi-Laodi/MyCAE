#pragma once

#include "commands/WorkflowCommandContext.h"
#include "workflow/FaceGroupWorkflowController.h"

class ProjectWorkflowController;

QString selectedFaceGroupId(const WorkflowCommandContext &context);
FaceGroupWorkflowController makeFaceGroupWorkflow(
    const WorkflowCommandContext &context,
    ProjectWorkflowController &projectWorkflow
);
