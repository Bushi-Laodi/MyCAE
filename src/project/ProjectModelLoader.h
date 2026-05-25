#pragma once

#include "geometry/GeometryManager.h"

#include <QString>

class ProjectModel;

class ProjectModelLoader
{
public:
    explicit ProjectModelLoader(const GeometryManager &geometryManager);

    bool loadGeometries(ProjectModel &projectModel, QString *errorMessage = nullptr) const;
    bool loadMeshes(ProjectModel &projectModel, QString *errorMessage = nullptr) const;
    bool loadSimulationCase(ProjectModel &projectModel, QString *errorMessage = nullptr) const;
    bool loadResults(ProjectModel &projectModel, QString *errorMessage = nullptr) const;

private:
    const GeometryManager &m_geometryManager;
};
