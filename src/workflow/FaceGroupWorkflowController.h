#pragma once

#include "geometry/FaceGroupService.h"

#include <QString>
#include <QStringList>

class ProjectWorkflowController;
class ProjectModel;
class PropertyPanel;
class RenderView;

struct FaceGroupWorkflowResult
{
    bool success = false;
    QString faceGroupId;
    QStringList logMessages;
};

class FaceGroupWorkflowController
{
public:
    FaceGroupWorkflowController(
        ProjectModel &projectModel,
        ProjectWorkflowController &projectWorkflow,
        PropertyPanel *propertyPanel,
        RenderView *renderView
    );

    FaceGroupWorkflowResult createOrReplacePickedFaces(
        const QString &geometryName,
        const QString &faceGroupName,
        const std::vector<PickSelection> &selections
    ) const;
    FaceGroupWorkflowResult appendPickedFaces(
        const QString &faceGroupId,
        const std::vector<PickSelection> &selections
    ) const;
    FaceGroupWorkflowResult removePickedFaces(
        const QString &faceGroupId,
        const std::vector<PickSelection> &selections
    ) const;
    FaceGroupWorkflowResult clearFaces(const QString &faceGroupId) const;
    FaceGroupWorkflowResult renameFaceGroup(const QString &faceGroupId, const QString &newName) const;
    FaceGroupWorkflowResult deleteFaceGroup(const QString &faceGroupId) const;
    FaceGroupWorkflowResult setLocalMeshSize(const QString &faceGroupId, double localMeshSize) const;
    FaceGroupWorkflowResult setPhysicalGroupEnabled(const QString &faceGroupId, bool enabled) const;
    FaceGroupReferenceReport referenceReport(const QString &faceGroupId) const;

private:
    FaceGroupWorkflowResult handleServiceResult(
        const FaceGroupServiceResult &serviceResult,
        bool selectFaceGroup
    ) const;

    ProjectModel &m_projectModel;
    ProjectWorkflowController &m_projectWorkflow;
    PropertyPanel *m_propertyPanel = nullptr;
    RenderView *m_renderView = nullptr;
};
