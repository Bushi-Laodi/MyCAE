#include "commands/MeshCommands.h"

#include "commands/CommandUtilities.h"
#include "project/ProjectModel.h"
#include "ui/MeshSetupDialog.h"
#include "workflow/MeshWorkflowController.h"
#include "workflow/ProjectWorkflowController.h"

#include <QMainWindow>

#include <optional>

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
        bool meshSetupChanged = false;

        switch (m_type) {
        case MeshCommandType::CheckGmsh:
            result = meshWorkflow.checkGmsh();
            break;
        case MeshCommandType::Generate:
            if (m_context.projectModel.hasProject()) {
                const std::optional<MeshSetup> meshSetup =
                    MeshSetupDialog::editMeshSetup(
                        m_context.window,
                        m_context.projectModel.meshRepository().meshSetup()
                    );
                if (!meshSetup) {
                    result.logMessages.append("Generate mesh canceled.");
                    break;
                }
                m_context.projectModel.meshRepository().meshSetup() = *meshSetup;
                meshSetupChanged = true;
            }
            result = meshWorkflow.generateMesh(m_context.projectModel);
            break;
        case MeshCommandType::ReadInfo:
            result = meshWorkflow.readMeshInfo(m_context.projectModel);
            break;
        case MeshCommandType::Show:
            result = meshWorkflow.showSelectedGeometryMesh(m_context.projectModel, m_context.renderView);
            break;
        }

        writeLogMessages(m_context, result.logMessages);

        ProjectWorkflowController projectWorkflow = makeProjectWorkflow(m_context);
        if (result.meshTreeChanged) {
            projectWorkflow.refreshMeshTree();
        }
        if (meshSetupChanged) {
            result.simulationCaseChanged = true;
        }
        if (result.simulationCaseChanged) {
            writeLogMessages(m_context, projectWorkflow.saveSimulationCase().logMessages);
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
