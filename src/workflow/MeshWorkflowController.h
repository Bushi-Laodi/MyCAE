#pragma once

#include "mesh/MeshObject.h"

#include <QStringList>

class ProjectModel;
class RenderView;

struct MeshWorkflowResult
{
    bool meshTreeChanged = false;
    bool faceGroupTreeChanged = false;
    bool resultTreeChanged = false;
    bool simulationCaseChanged = false;
    QStringList logMessages;
};

class MeshWorkflowController
{
public:
    MeshWorkflowResult checkGmsh() const;
    MeshWorkflowResult setLocalMeshSize(ProjectModel &projectModel, const QString &faceGroupId, double localMeshSize) const;
    MeshWorkflowResult generateMesh(ProjectModel &projectModel) const;
    MeshWorkflowResult readMeshInfo(ProjectModel &projectModel) const;
    MeshWorkflowResult showSelectedGeometryMesh(const ProjectModel &projectModel, RenderView *renderView) const;
    MeshWorkflowResult displayMeshObject(
        const ProjectModel &projectModel,
        const MeshObject &meshObject,
        RenderView *renderView
    ) const;
};
