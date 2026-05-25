#include "geometry/FaceGroupReferenceService.h"

#include "project/ProjectModel.h"
#include "solver/BoundaryCondition.h"

void FaceGroupReferenceService::updateBoundaryConditionReferences(
    ProjectModel &projectModel,
    const QString &oldFaceGroupId,
    const QString &newFaceGroupId,
    const QString &newFaceGroupName
)
{
    for (BoundaryCondition &boundaryCondition : projectModel.boundaryConditions()) {
        if (boundaryCondition.target.faceGroupId != oldFaceGroupId) {
            continue;
        }
        boundaryCondition.target.faceGroupId = newFaceGroupId;
        boundaryCondition.target.faceGroupName = newFaceGroupName;
    }
}

FaceGroupReferenceReport FaceGroupReferenceService::validateFaceGroupReferences(
    const ProjectModel &projectModel,
    const QString &faceGroupId
)
{
    FaceGroupReferenceReport report;
    for (const BoundaryCondition &boundaryCondition : projectModel.boundaryConditions()) {
        if (boundaryCondition.target.faceGroupId != faceGroupId) {
            continue;
        }
        ++report.boundaryConditionCount;
        report.boundaryConditionNames.append(boundaryCondition.name);
    }
    return report;
}
