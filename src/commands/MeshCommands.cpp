#include "commands/MeshCommands.h"

#include "commands/CommandUtilities.h"
#include "workflow/MeshWorkflowController.h"
#include "workflow/ProjectWorkflowController.h"

namespace
{
class MeshCommand final : public AppCommand
{
public:
    MeshCommand(WorkflowCommandContext context, MeshCommandType type)
        : m_context(context)
        , m_type(type)
    {
    }

    void execute() override
    {
        const MeshWorkflowController meshWorkflow;
        MeshWorkflowResult result;

        switch (m_type) {
        case MeshCommandType::CheckGmsh:
            result = meshWorkflow.checkGmsh();
            break;
        case MeshCommandType::Generate:
            result = meshWorkflow.generateMesh(m_context.projectModel);
            break;
        case MeshCommandType::ReadInfo:
            result = meshWorkflow.readMeshInfo(m_context.projectModel);
            break;
        case MeshCommandType::Show:
            result = meshWorkflow.showSelectedGeometryMesh(m_context.projectModel, m_context.renderView);
            break;
        }

        writeLogMessages(m_context.logPanel, result.logMessages);

        ProjectWorkflowController projectWorkflow = makeProjectWorkflow(m_context);
        if (result.meshTreeChanged) {
            projectWorkflow.refreshMeshTree();
        }
        if (result.simulationCaseChanged) {
            writeLogMessages(m_context.logPanel, projectWorkflow.saveSimulationCase().logMessages);
        }
    }

private:
    WorkflowCommandContext m_context;
    MeshCommandType m_type;
};
}

std::unique_ptr<AppCommand> makeMeshCommand(WorkflowCommandContext context, MeshCommandType type)
{
    return std::make_unique<MeshCommand>(context, type);
}
