#include "commands/SolverCommands.h"

#include "commands/CommandUtilities.h"
#include "workflow/ProjectWorkflowController.h"
#include "workflow/SolverWorkflowController.h"

#include <utility>

namespace
{
class SolverDataCommand final : public AppCommand
{
public:
    SolverDataCommand(WorkflowCommandContext context, SolverDataCommandType type)
        : m_context(context)
        , m_type(type)
    {
    }

    void execute() override
    {
        ProjectWorkflowController projectWorkflow = makeProjectWorkflow(m_context);
        SolverWorkflowController solverWorkflow = makeSolverWorkflow(m_context, projectWorkflow);
        SolverWorkflowResult result;

        switch (m_type) {
        case SolverDataCommandType::CreateMaterial:
            result = solverWorkflow.createMaterial();
            break;
        case SolverDataCommandType::CreateBoundaryCondition:
            result = solverWorkflow.createBoundaryCondition();
            break;
        case SolverDataCommandType::CreateLoad:
            result = solverWorkflow.createLoad();
            break;
        case SolverDataCommandType::EditSelected:
            result = solverWorkflow.editSelectedData();
            break;
        case SolverDataCommandType::DeleteSelected:
            result = solverWorkflow.deleteSelectedData();
            break;
        }

        writeLogMessages(m_context, result.logMessages);
    }

private:
    WorkflowCommandContext m_context;
    SolverDataCommandType m_type;
};

class RunSolverCommand final : public AppCommand
{
public:
    RunSolverCommand(WorkflowCommandContext context, QString pluginId)
        : m_context(context)
        , m_pluginId(std::move(pluginId))
    {
    }

    void execute() override
    {
        ProjectWorkflowController projectWorkflow = makeProjectWorkflow(m_context);
        const SolverWorkflowResult result =
            makeSolverWorkflow(m_context, projectWorkflow).runSolverPlugin(m_pluginId);
        writeLogMessages(m_context, result.logMessages);
    }

private:
    WorkflowCommandContext m_context;
    QString m_pluginId;
};
}

std::unique_ptr<AppCommand> makeSolverDataCommand(WorkflowCommandContext context, SolverDataCommandType type)
{
    return std::make_unique<SolverDataCommand>(context, type);
}

std::unique_ptr<AppCommand> makeRunSolverCommand(WorkflowCommandContext context, const QString &pluginId)
{
    return std::make_unique<RunSolverCommand>(context, pluginId);
}
