#include "commands/FaceGroupEditCommands.h"

#include "commands/CommandUtilities.h"
#include "commands/FaceGroupCommandUtilities.h"
#include "geometry/FaceGroup.h"
#include "picking/PickController.h"
#include "project/ProjectModel.h"
#include "workflow/ProjectWorkflowController.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>

namespace
{
class FaceGroupEditCommand final : public AppCommand
{
public:
    FaceGroupEditCommand(WorkflowCommandContext context, FaceGroupEditCommandType type)
        : m_context(context)
        , m_type(type)
    {
    }

    void execute() override
    {
        if (!m_context.pickController) {
            writeLogMessages(m_context.logPanel, {"Face group command failed: pick controller is not available."});
            return;
        }

        ProjectWorkflowController projectWorkflow = makeProjectWorkflow(m_context);
        FaceGroupWorkflowController workflow = makeFaceGroupWorkflow(m_context, projectWorkflow);
        switch (m_type) {
        case FaceGroupEditCommandType::CreateFromPick:
            createFromPick(workflow);
            break;
        case FaceGroupEditCommandType::AddPickedFaces:
            updateSelectedFaceGroup(workflow, m_type);
            break;
        case FaceGroupEditCommandType::RemovePickedFaces:
            updateSelectedFaceGroup(workflow, m_type);
            break;
        case FaceGroupEditCommandType::ClearFaces:
            updateSelectedFaceGroup(workflow, m_type);
            break;
        case FaceGroupEditCommandType::Rename:
            updateSelectedFaceGroup(workflow, m_type);
            break;
        case FaceGroupEditCommandType::Delete:
            updateSelectedFaceGroup(workflow, m_type);
            break;
        case FaceGroupEditCommandType::SetLocalMeshSize:
            updateSelectedFaceGroup(workflow, m_type);
            break;
        case FaceGroupEditCommandType::TogglePhysicalGroup:
            updateSelectedFaceGroup(workflow, m_type);
            break;
        }
    }

private:
    void createFromPick(const FaceGroupWorkflowController &workflow) const
    {
        bool accepted = false;
        const QString faceGroupName = QInputDialog::getText(
            m_context.window,
            "Create Face Group",
            "Face group name:",
            QLineEdit::Normal,
            "PickedFaces",
            &accepted
        );
        if (!accepted) {
            writeLogMessages(m_context.logPanel, {"Create face group canceled."});
            return;
        }

        const FaceGroupWorkflowResult result = workflow.createOrReplacePickedFaces(
            m_context.pickController->geometryName(),
            faceGroupName,
            m_context.pickController->selections()
        );
        writeLogMessages(m_context.logPanel, result.logMessages);
    }

    void updateSelectedFaceGroup(
        const FaceGroupWorkflowController &workflow,
        FaceGroupEditCommandType type
    ) const
    {
        const QString faceGroupId = selectedFaceGroupId(m_context);
        if (faceGroupId.isEmpty()) {
            writeLogMessages(m_context.logPanel, {"Face group operation failed: select a face group first."});
            return;
        }

        FaceGroupWorkflowResult result;
        switch (type) {
        case FaceGroupEditCommandType::AddPickedFaces:
            result = workflow.appendPickedFaces(faceGroupId, m_context.pickController->selections());
            break;
        case FaceGroupEditCommandType::RemovePickedFaces:
            result = workflow.removePickedFaces(faceGroupId, m_context.pickController->selections());
            break;
        case FaceGroupEditCommandType::ClearFaces:
            if (QMessageBox::question(m_context.window, "Clear Face Group", "Clear all faces in \"" + faceGroupId + "\"?")
                != QMessageBox::Yes) {
                writeLogMessages(m_context.logPanel, {"Clear face group canceled."});
                return;
            }
            result = workflow.clearFaces(faceGroupId);
            break;
        case FaceGroupEditCommandType::Rename:
            result = renameFaceGroup(workflow, faceGroupId);
            break;
        case FaceGroupEditCommandType::Delete:
            result = deleteFaceGroup(workflow, faceGroupId);
            break;
        case FaceGroupEditCommandType::SetLocalMeshSize:
            result = setLocalMeshSize(workflow, faceGroupId);
            break;
        case FaceGroupEditCommandType::TogglePhysicalGroup:
            result = togglePhysicalGroup(workflow, faceGroupId);
            break;
        case FaceGroupEditCommandType::CreateFromPick:
            break;
        }

        writeLogMessages(m_context.logPanel, result.logMessages);
    }

    FaceGroupWorkflowResult renameFaceGroup(
        const FaceGroupWorkflowController &workflow,
        const QString &faceGroupId
    ) const
    {
        bool accepted = false;
        const QString newName = QInputDialog::getText(
            m_context.window,
            "Rename Face Group",
            "New face group name:",
            QLineEdit::Normal,
            FaceGroups::nameFromId(faceGroupId),
            &accepted
        );
        if (!accepted) {
            return {false, {}, {"Rename face group canceled."}};
        }
        return workflow.renameFaceGroup(faceGroupId, newName);
    }

    FaceGroupWorkflowResult deleteFaceGroup(
        const FaceGroupWorkflowController &workflow,
        const QString &faceGroupId
    ) const
    {
        const FaceGroupReferenceReport report = workflow.referenceReport(faceGroupId);
        QString message = "Delete face group \"" + faceGroupId + "\"?";
        if (report.hasReferences()) {
            message += "\n\nBoundary conditions referencing it will lose their face group target:\n"
                + report.boundaryConditionNames.join(", ");
        }
        if (QMessageBox::question(m_context.window, "Delete Face Group", message) != QMessageBox::Yes) {
            return {false, {}, {"Delete face group canceled."}};
        }
        return workflow.deleteFaceGroup(faceGroupId);
    }

    FaceGroupWorkflowResult setLocalMeshSize(
        const FaceGroupWorkflowController &workflow,
        const QString &faceGroupId
    ) const
    {
        bool accepted = false;
        const double value = QInputDialog::getDouble(
            m_context.window,
            "Set Local Mesh Size",
            "Local mesh size (0 disables local mesh):",
            0.0,
            0.0,
            1.0e9,
            6,
            &accepted
        );
        if (!accepted) {
            return {false, {}, {"Set local mesh size canceled."}};
        }
        return workflow.setLocalMeshSize(faceGroupId, value);
    }

    FaceGroupWorkflowResult togglePhysicalGroup(
        const FaceGroupWorkflowController &workflow,
        const QString &faceGroupId
    ) const
    {
        const FaceGroup *faceGroup = m_context.projectModel.findFaceGroupById(faceGroupId);
        if (!faceGroup) {
            return {false, {}, {"Toggle physical group failed: face group not found."}};
        }
        return workflow.setPhysicalGroupEnabled(faceGroupId, !faceGroup->physicalGroupEnabled);
    }

    WorkflowCommandContext m_context;
    FaceGroupEditCommandType m_type;
};
}

std::unique_ptr<AppCommand> makeFaceGroupEditCommand(
    WorkflowCommandContext context,
    FaceGroupEditCommandType type
)
{
    return std::make_unique<FaceGroupEditCommand>(context, type);
}
