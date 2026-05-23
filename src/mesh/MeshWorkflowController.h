#pragma once

#include "mesh/MeshObject.h"

#include <QStringList>

class ProjectModel;
class RenderView;

struct MeshWorkflowResult
{
    bool meshTreeChanged = false;
    bool simulationCaseChanged = false;
    QStringList logMessages;
};

class MeshWorkflowController
{
public:
    MeshWorkflowResult checkGmsh() const;
    MeshWorkflowResult generateMesh(ProjectModel &projectModel) const;
    MeshWorkflowResult readMeshInfo(const ProjectModel &projectModel) const;
    MeshWorkflowResult showSelectedGeometryMesh(const ProjectModel &projectModel, RenderView *renderView) const;
    MeshWorkflowResult displayMeshObject(
        const ProjectModel &projectModel,
        const MeshObject &meshObject,
        RenderView *renderView
    ) const;
};
