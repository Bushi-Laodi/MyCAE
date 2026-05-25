#include "commands/FaceGroupCommandUtilities.h"

#include "project/ProjectModel.h"
#include "project/SelectionState.h"
#include "workflow/ProjectWorkflowController.h"

QString selectedFaceGroupId(const WorkflowCommandContext &context)
{
    const Selection &selection = context.projectModel.selection();
    return selection.kind == SelectionKind::FaceGroup ? selection.id : QString();
}

FaceGroupWorkflowController makeFaceGroupWorkflow(
    const WorkflowCommandContext &context,
    ProjectWorkflowController &projectWorkflow
)
{
    return FaceGroupWorkflowController(
        context.projectModel,
        projectWorkflow,
        context.propertyPanel,
        context.renderView
    );
}
