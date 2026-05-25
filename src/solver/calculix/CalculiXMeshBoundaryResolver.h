#pragma once

#include "mesh/MeshBoundary.h"
#include "solver/calculix/CalculiXCaseData.h"

#include <QStringList>

#include <vector>

class ProjectModel;

struct CalculiXMeshBoundaryResolveResult
{
    std::vector<MeshBoundary> meshBoundaries;
    QStringList warnings;
    QStringList logMessages;
};

class CalculiXMeshBoundaryResolver
{
public:
    CalculiXMeshBoundaryResolveResult resolve(
        const ProjectModel &projectModel,
        const CalculiXCaseData &caseData
    ) const;
};
