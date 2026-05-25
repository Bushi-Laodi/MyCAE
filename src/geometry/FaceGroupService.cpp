#include "geometry/FaceGroupService.h"

#include "geometry/FaceGroupReferenceService.h"
#include "geometry/FaceReferenceUtils.h"
#include "project/ProjectModel.h"

#include <algorithm>

FaceGroupServiceResult FaceGroupService::createOrReplacePickedFaces(
    ProjectModel &projectModel,
    const QString &geometryName,
    const QString &faceGroupName,
    const std::vector<PickSelection> &selections
)
{
    FaceGroupServiceResult result;
    const QString geometry = geometryName.trimmed();
    GeometryRepository &geometryRepository = projectModel.geometryRepository();
    SolverRepository &solverRepository = projectModel.solverRepository();
    if (geometry.isEmpty() || !geometryRepository.findGeometryByName(geometry)) {
        result.logMessages.append("Face group save failed: geometry is not available.");
        return result;
    }

    const std::vector<FaceReference> references = FaceReferenceUtils::normalizedReferences(selections);
    if (references.empty()) {
        result.logMessages.append("Face group save failed: no valid faces were picked.");
        return result;
    }

    const QString name = FaceReferenceUtils::normalizedFaceGroupName(faceGroupName);
    const QString id = FaceGroups::makeId(geometry, name);
    FaceGroup *existingFaceGroup = solverRepository.findFaceGroupById(id);
    if (existingFaceGroup) {
        existingFaceGroup->name = name;
        existingFaceGroup->geometryName = geometry;
        existingFaceGroup->role = existingFaceGroup->role.trimmed().isEmpty() ? "Boundary" : existingFaceGroup->role;
        FaceReferenceUtils::replaceReferences(*existingFaceGroup, references);
        result.logMessages.append(
            QString("Face group replaced: %1 now has %2 face(s).")
                .arg(existingFaceGroup->id)
                .arg(existingFaceGroup->faceIndices.size())
        );
    } else {
        FaceGroup faceGroup;
        faceGroup.id = id;
        faceGroup.name = name;
        faceGroup.geometryName = geometry;
        faceGroup.role = "Boundary";
        FaceReferenceUtils::replaceReferences(faceGroup, references);
        solverRepository.faceGroups().push_back(faceGroup);
        result.logMessages.append(
            QString("Face group created: %1 with %2 face(s).")
                .arg(faceGroup.id)
                .arg(faceGroup.faceIndices.size())
        );
    }

    result.success = true;
    result.faceGroupId = id;
    return result;
}

FaceGroupServiceResult FaceGroupService::appendPickedFaces(
    ProjectModel &projectModel,
    const QString &faceGroupId,
    const std::vector<PickSelection> &selections
)
{
    FaceGroupServiceResult result;
    FaceGroup *faceGroup = projectModel.solverRepository().findFaceGroupById(faceGroupId);
    if (!faceGroup) {
        result.logMessages.append("Append picked faces failed: face group not found: " + faceGroupId);
        return result;
    }

    const std::vector<FaceReference> references = FaceReferenceUtils::normalizedReferences(selections);
    if (references.empty()) {
        result.logMessages.append("Append picked faces failed: no valid picked faces.");
        return result;
    }

    FaceReferenceUtils::appendReferences(*faceGroup, references);
    result.success = true;
    result.faceGroupId = faceGroup->id;
    result.logMessages.append(
        QString("Face group updated: %1 now has %2 face(s).")
            .arg(faceGroup->id)
            .arg(faceGroup->faceIndices.size())
    );
    return result;
}

FaceGroupServiceResult FaceGroupService::removePickedFaces(
    ProjectModel &projectModel,
    const QString &faceGroupId,
    const std::vector<PickSelection> &selections
)
{
    FaceGroupServiceResult result;
    FaceGroup *faceGroup = projectModel.solverRepository().findFaceGroupById(faceGroupId);
    if (!faceGroup) {
        result.logMessages.append("Remove picked faces failed: face group not found: " + faceGroupId);
        return result;
    }

    const std::vector<FaceReference> references = FaceReferenceUtils::normalizedReferences(selections);
    if (references.empty()) {
        result.logMessages.append("Remove picked faces failed: no valid picked faces.");
        return result;
    }

    const int removedCount = FaceReferenceUtils::removeReferences(*faceGroup, references);
    result.success = true;
    result.faceGroupId = faceGroup->id;
    result.logMessages.append(
        QString("Face group updated: removed %1 face(s), %2 face(s) remain.")
            .arg(removedCount)
            .arg(faceGroup->faceIndices.size())
    );
    return result;
}

FaceGroupServiceResult FaceGroupService::clearFaces(ProjectModel &projectModel, const QString &faceGroupId)
{
    FaceGroupServiceResult result;
    FaceGroup *faceGroup = projectModel.solverRepository().findFaceGroupById(faceGroupId);
    if (!faceGroup) {
        result.logMessages.append("Clear face group failed: face group not found: " + faceGroupId);
        return result;
    }

    faceGroup->faceReferences.clear();
    faceGroup->faceIndices.clear();
    result.success = true;
    result.faceGroupId = faceGroup->id;
    result.logMessages.append("Face group faces cleared: " + faceGroup->id);
    return result;
}

FaceGroupServiceResult FaceGroupService::renameFaceGroup(
    ProjectModel &projectModel,
    const QString &faceGroupId,
    const QString &newName
)
{
    FaceGroupServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    FaceGroup *faceGroup = solverRepository.findFaceGroupById(faceGroupId);
    if (!faceGroup) {
        result.logMessages.append("Rename face group failed: face group not found: " + faceGroupId);
        return result;
    }

    const QString normalizedName = FaceReferenceUtils::normalizedFaceGroupName(newName);
    const QString newId = FaceGroups::makeId(faceGroup->geometryName, normalizedName);
    if (newId != faceGroupId && solverRepository.findFaceGroupById(newId)) {
        result.logMessages.append("Rename face group failed: target name already exists: " + newId);
        return result;
    }

    faceGroup->id = newId;
    faceGroup->name = normalizedName;
    FaceGroupReferenceService::updateBoundaryConditionReferences(projectModel, faceGroupId, newId, normalizedName);
    result.success = true;
    result.faceGroupId = faceGroupId;
    result.newFaceGroupId = newId;
    result.logMessages.append("Face group renamed: " + faceGroupId + " -> " + newId);
    return result;
}

FaceGroupServiceResult FaceGroupService::deleteFaceGroup(ProjectModel &projectModel, const QString &faceGroupId)
{
    FaceGroupServiceResult result;
    auto &faceGroups = projectModel.solverRepository().faceGroups();
    const auto oldSize = faceGroups.size();
    faceGroups.erase(
        std::remove_if(faceGroups.begin(), faceGroups.end(), [&faceGroupId](const FaceGroup &faceGroup) {
            return faceGroup.id == faceGroupId;
        }),
        faceGroups.end()
    );

    if (faceGroups.size() == oldSize) {
        result.logMessages.append("Delete face group failed: face group not found: " + faceGroupId);
        return result;
    }

    FaceGroupReferenceService::updateBoundaryConditionReferences(projectModel, faceGroupId, {}, {});
    result.success = true;
    result.faceGroupId = faceGroupId;
    result.logMessages.append("Face group deleted: " + faceGroupId);
    return result;
}

FaceGroupServiceResult FaceGroupService::setLocalMeshSize(
    ProjectModel &projectModel,
    const QString &faceGroupId,
    double localMeshSize
)
{
    FaceGroupServiceResult result;
    FaceGroup *faceGroup = projectModel.solverRepository().findFaceGroupById(faceGroupId);
    if (!faceGroup) {
        result.logMessages.append("Set local mesh size failed: face group not found: " + faceGroupId);
        return result;
    }

    faceGroup->localMeshSize = localMeshSize;
    faceGroup->localMeshEnabled = localMeshSize > 0.0;
    result.success = true;
    result.faceGroupId = faceGroup->id;
    result.logMessages.append(
        faceGroup->localMeshEnabled
            ? QString("Local mesh size set for %1: %2").arg(faceGroup->id).arg(faceGroup->localMeshSize)
            : QString("Local mesh disabled for %1.").arg(faceGroup->id)
    );
    return result;
}

FaceGroupServiceResult FaceGroupService::setPhysicalGroupEnabled(
    ProjectModel &projectModel,
    const QString &faceGroupId,
    bool enabled
)
{
    FaceGroupServiceResult result;
    FaceGroup *faceGroup = projectModel.solverRepository().findFaceGroupById(faceGroupId);
    if (!faceGroup) {
        result.logMessages.append("Set physical group failed: face group not found: " + faceGroupId);
        return result;
    }

    faceGroup->physicalGroupEnabled = enabled;
    result.success = true;
    result.faceGroupId = faceGroup->id;
    result.logMessages.append(
        QString("Physical group %1 for %2.")
            .arg(enabled ? "enabled" : "disabled")
            .arg(faceGroup->id)
    );
    return result;
}

FaceGroupReferenceReport FaceGroupService::validateFaceGroupReferences(
    const ProjectModel &projectModel,
    const QString &faceGroupId
)
{
    return FaceGroupReferenceService::validateFaceGroupReferences(projectModel, faceGroupId);
}
