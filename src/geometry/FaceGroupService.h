#pragma once

#include "geometry/FaceGroup.h"
#include "picking/PickSelection.h"

#include <QString>
#include <QStringList>

#include <vector>

class ProjectModel;

struct FaceGroupServiceResult
{
    bool success = false;
    QString faceGroupId;
    QString newFaceGroupId;
    QStringList logMessages;
};

struct FaceGroupReferenceReport
{
    int boundaryConditionCount = 0;
    QStringList boundaryConditionNames;

    bool hasReferences() const
    {
        return boundaryConditionCount > 0;
    }
};

class FaceGroupService
{
public:
    static FaceGroupServiceResult createOrReplacePickedFaces(
        ProjectModel &projectModel,
        const QString &geometryName,
        const QString &faceGroupName,
        const std::vector<PickSelection> &selections
    );
    static FaceGroupServiceResult appendPickedFaces(
        ProjectModel &projectModel,
        const QString &faceGroupId,
        const std::vector<PickSelection> &selections
    );
    static FaceGroupServiceResult removePickedFaces(
        ProjectModel &projectModel,
        const QString &faceGroupId,
        const std::vector<PickSelection> &selections
    );
    static FaceGroupServiceResult clearFaces(ProjectModel &projectModel, const QString &faceGroupId);
    static FaceGroupServiceResult renameFaceGroup(
        ProjectModel &projectModel,
        const QString &faceGroupId,
        const QString &newName
    );
    static FaceGroupServiceResult deleteFaceGroup(ProjectModel &projectModel, const QString &faceGroupId);
    static FaceGroupServiceResult setLocalMeshSize(
        ProjectModel &projectModel,
        const QString &faceGroupId,
        double localMeshSize
    );
    static FaceGroupServiceResult setPhysicalGroupEnabled(
        ProjectModel &projectModel,
        const QString &faceGroupId,
        bool enabled
    );
    static FaceGroupReferenceReport validateFaceGroupReferences(
        const ProjectModel &projectModel,
        const QString &faceGroupId
    );
};
