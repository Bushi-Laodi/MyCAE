#include "commands/CommandUtilities.h"

#include "ui/LogPanel.h"
#include "workflow/GeometryWorkflowController.h"
#include "workflow/ProjectWorkflowController.h"
#include "workflow/SolverWorkflowController.h"

#include <QMainWindow>

void writeLogMessages(LogPanel *logPanel, const QStringList &messages)
{
    if (!logPanel) {
        return;
    }

    for (const QString &message : messages) {
        logPanel->appendMessage(message);
    }
}

void writeLogMessages(const WorkflowCommandContext &context, const QStringList &messages)
{
    if (context.writeLogMessages) {
        context.writeLogMessages(messages);
        return;
    }
    writeLogMessages(context.logPanel, messages);
}

ProjectWorkflowController makeProjectWorkflow(const WorkflowCommandContext &context)
{
    return ProjectWorkflowController(
        context.projectManager,
        context.geometryManager,
        context.projectModel,
        context.projectTreePanel,
        context.propertyPanel,
        context.renderView,
        context.window
    );
}

GeometryWorkflowController makeGeometryWorkflow(
    const WorkflowCommandContext &context,
    ProjectWorkflowController &projectWorkflow
)
{
    return GeometryWorkflowController(
        context.geometryManager,
        context.projectModel,
        projectWorkflow,
        context.propertyPanel,
        context.renderView,
        context.window
    );
}

SolverWorkflowController makeSolverWorkflow(
    const WorkflowCommandContext &context,
    ProjectWorkflowController &projectWorkflow
)
{
    return SolverWorkflowController(
        context.projectModel,
        context.solverPluginManager,
        projectWorkflow,
        context.propertyPanel,
        context.renderView,
        context.window
    );
}
