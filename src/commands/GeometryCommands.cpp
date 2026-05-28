#include "commands/GeometryCommands.h"

#include "commands/CommandUtilities.h"
#include "picking/PickController.h"
#include "workflow/GeometryWorkflowController.h"
#include "workflow/ProjectWorkflowController.h"

namespace
{
class GeometryCreateCommand final : public AppCommand
{
public:
    GeometryCreateCommand(WorkflowCommandContext context, GeometryCreateType type)
        : m_context(context)
        , m_type(type)
    {
    }

    void execute() override
    {
        ProjectWorkflowController projectWorkflow = makeProjectWorkflow(m_context);
        const GeometryWorkflowResult result = makeGeometryWorkflow(m_context, projectWorkflow).createGeometry(m_type);
        if (result.success && m_context.pickController) {
            m_context.pickController->clear(m_context.renderView);
        }
        writeLogMessages(m_context.logPanel, result.logMessages);
    }

private:
    WorkflowCommandContext m_context;
    GeometryCreateType m_type;
};

class GeometryBooleanCommand final : public AppCommand
{
public:
    explicit GeometryBooleanCommand(WorkflowCommandContext context)
        : m_context(context)
    {
    }

    void execute() override
    {
        ProjectWorkflowController projectWorkflow = makeProjectWorkflow(m_context);
        const GeometryWorkflowResult result = makeGeometryWorkflow(m_context, projectWorkflow).createBooleanGeometry();
        if (result.success && m_context.pickController) {
            m_context.pickController->clear(m_context.renderView);
        }
        writeLogMessages(m_context.logPanel, result.logMessages);
    }

private:
    WorkflowCommandContext m_context;
};
}

std::unique_ptr<AppCommand> makeGeometryCreateCommand(WorkflowCommandContext context, GeometryCreateType type)
{
    return std::make_unique<GeometryCreateCommand>(context, type);
}

std::unique_ptr<AppCommand> makeGeometryBooleanCommand(WorkflowCommandContext context)
{
    return std::make_unique<GeometryBooleanCommand>(context);
}
