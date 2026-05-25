#include "workflow/FaceGroupWorkflowController.h"

#include "project/SelectionState.h"
#include "workflow/ProjectWorkflowController.h"
#include "workflow/SelectionController.h"

FaceGroupWorkflowController::FaceGroupWorkflowController(
    ProjectModel &projectModel,
    ProjectWorkflowController &projectWorkflow,
    PropertyPanel *propertyPanel,
    RenderView *renderView
)
    : m_projectModel(projectModel)
    , m_projectWorkflow(projectWorkflow)
    , m_propertyPanel(propertyPanel)
    , m_renderView(renderView)
{
}

FaceGroupWorkflowResult FaceGroupWorkflowController::createOrReplacePickedFaces(
    const QString &geometryName,
    const QString &faceGroupName,
    const std::vector<PickSelection> &selections
) const
{
    return handleServiceResult(
        FaceGroupService::createOrReplacePickedFaces(m_projectModel, geometryName, faceGroupName, selections),
        true
    );
}

FaceGroupWorkflowResult FaceGroupWorkflowController::appendPickedFaces(
    const QString &faceGroupId,
    const std::vector<PickSelection> &selections
) const
{
    return handleServiceResult(FaceGroupService::appendPickedFaces(m_projectModel, faceGroupId, selections), true);
}

FaceGroupWorkflowResult FaceGroupWorkflowController::removePickedFaces(
    const QString &faceGroupId,
    const std::vector<PickSelection> &selections
) const
{
    return handleServiceResult(FaceGroupService::removePickedFaces(m_projectModel, faceGroupId, selections), true);
}

FaceGroupWorkflowResult FaceGroupWorkflowController::clearFaces(const QString &faceGroupId) const
{
    return handleServiceResult(FaceGroupService::clearFaces(m_projectModel, faceGroupId), true);
}

FaceGroupWorkflowResult FaceGroupWorkflowController::renameFaceGroup(
    const QString &faceGroupId,
    const QString &newName
) const
{
    return handleServiceResult(FaceGroupService::renameFaceGroup(m_projectModel, faceGroupId, newName), true);
}

FaceGroupWorkflowResult FaceGroupWorkflowController::deleteFaceGroup(const QString &faceGroupId) const
{
    return handleServiceResult(FaceGroupService::deleteFaceGroup(m_projectModel, faceGroupId), false);
}

FaceGroupWorkflowResult FaceGroupWorkflowController::setLocalMeshSize(
    const QString &faceGroupId,
    double localMeshSize
) const
{
    return handleServiceResult(FaceGroupService::setLocalMeshSize(m_projectModel, faceGroupId, localMeshSize), true);
}

FaceGroupWorkflowResult FaceGroupWorkflowController::setPhysicalGroupEnabled(
    const QString &faceGroupId,
    bool enabled
) const
{
    return handleServiceResult(FaceGroupService::setPhysicalGroupEnabled(m_projectModel, faceGroupId, enabled), true);
}

FaceGroupReferenceReport FaceGroupWorkflowController::referenceReport(const QString &faceGroupId) const
{
    return FaceGroupService::validateFaceGroupReferences(m_projectModel, faceGroupId);
}

FaceGroupWorkflowResult FaceGroupWorkflowController::handleServiceResult(
    const FaceGroupServiceResult &serviceResult,
    bool selectFaceGroup
) const
{
    FaceGroupWorkflowResult result;
    result.logMessages.append(serviceResult.logMessages);
    if (!serviceResult.success) {
        return result;
    }

    const QString faceGroupId = serviceResult.newFaceGroupId.isEmpty()
        ? serviceResult.faceGroupId
        : serviceResult.newFaceGroupId;

    m_projectWorkflow.refreshFaceGroupTree();
    result.logMessages.append(m_projectWorkflow.saveSimulationCase().logMessages);

    if (selectFaceGroup && !faceGroupId.isEmpty()) {
        const SelectionController selectionController(m_projectModel, m_propertyPanel, m_renderView);
        result.logMessages.append(
            selectionController.apply(Selection::item(SelectionKind::FaceGroup, faceGroupId)).logMessages
        );
    }

    result.success = true;
    result.faceGroupId = faceGroupId;
    return result;
}
