#pragma once

#include "geometry/FaceGroupService.h"

class ProjectModel;

class FaceGroupReferenceService
{
public:
    static void updateBoundaryConditionReferences(
        ProjectModel &projectModel,
        const QString &oldFaceGroupId,
        const QString &newFaceGroupId,
        const QString &newFaceGroupName
    );
    static FaceGroupReferenceReport validateFaceGroupReferences(
        const ProjectModel &projectModel,
        const QString &faceGroupId
    );
};
