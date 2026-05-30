#include "commands/MeshCommands.h"

#include "commands/CommandUtilities.h"
#include "geometry/FaceGroup.h"
#include "geometry/GeometryObject.h"
#include "project/ProjectModel.h"
#include "ui/MeshSetupDialog.h"
#include "workflow/MeshWorkflowController.h"
#include "workflow/ProjectWorkflowController.h"

#include <QMainWindow>

#include <optional>

namespace
{
void appendWorkflowResult(MeshWorkflowResult &target, const MeshWorkflowResult &source)
{
    target.meshTreeChanged = target.meshTreeChanged || source.meshTreeChanged;
    target.faceGroupTreeChanged = target.faceGroupTreeChanged || source.faceGroupTreeChanged;
    target.resultTreeChanged = target.resultTreeChanged || source.resultTreeChanged;
    target.simulationCaseChanged = target.simulationCaseChanged || source.simulationCaseChanged;
    target.logMessages.append(source.logMessages);
}

MeshSetupDialogOptions meshSetupOptionsForSelection(const ProjectModel &projectModel)
{
    MeshSetupDialogOptions options;
    const GeometryObject *geometry = projectModel.geometryForSelection();
    if (!geometry) {
        return options;
    }

    options.geometryName = geometry->name;
    for (const FaceGroup &faceGroup : projectModel.solverRepository().faceGroups()) {
        if (faceGroup.geometryName == geometry->name) {
            options.faceGroups.push_back(faceGroup);
        }
    }
    return options;
}

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
                const std::optional<MeshSetupDialogResult> meshSetupResult =
                    MeshSetupDialog::editMeshSetup(
                        m_context.window,
                        m_context.projectModel.meshRepository().meshSetup(),
                        meshSetupOptionsForSelection(m_context.projectModel)
                    );
                if (!meshSetupResult) {
                    result.logMessages.append("Generate mesh canceled.");
                    break;
                }
                m_context.projectModel.meshRepository().meshSetup() = meshSetupResult->meshSetup;
                meshSetupChanged = true;
                for (const LocalMeshSizeChange &change : meshSetupResult->localMeshSizeChanges) {
                    appendWorkflowResult(
                        result,
                        meshWorkflow.setLocalMeshSize(m_context.projectModel, change.faceGroupId, change.localMeshSize)
                    );
                }
            }
            appendWorkflowResult(result, meshWorkflow.generateMesh(m_context.projectModel));
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
        if (result.faceGroupTreeChanged) {
            projectWorkflow.refreshFaceGroupTree();
        }
        if (result.meshTreeChanged) {
            projectWorkflow.refreshMeshTree();
        }
        if (result.resultTreeChanged) {
            projectWorkflow.refreshResultTree();
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
