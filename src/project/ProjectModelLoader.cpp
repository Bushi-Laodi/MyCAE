#include "ProjectModelLoader.h"

#include "mesh/MeshManager.h"
#include "project/ProjectModel.h"
#include "solver/SimulationCaseManager.h"

#include <vector>

ProjectModelLoader::ProjectModelLoader(const GeometryManager &geometryManager)
    : m_geometryManager(geometryManager)
{
}

bool ProjectModelLoader::loadGeometries(ProjectModel &projectModel, QString *errorMessage) const
{
    QVector<BoxGeometry> loadedBoxes;
    if (!m_geometryManager.loadBoxGeometries(projectModel.project(), &loadedBoxes, errorMessage)) {
        return false;
    }

    QVector<CylinderGeometry> loadedCylinders;
    if (!m_geometryManager.loadCylinderGeometries(projectModel.project(), &loadedCylinders, errorMessage)) {
        return false;
    }

    std::vector<GeometryObject> loadedGeometries;
    if (!m_geometryManager.loadGeometryObjects(projectModel.project(), loadedGeometries, errorMessage)) {
        return false;
    }

    projectModel.boxes() = loadedBoxes;
    projectModel.cylinders() = loadedCylinders;
    projectModel.geometryObjects().clear();
    for (const GeometryObject &geometry : loadedGeometries) {
        projectModel.geometryObjects().append(geometry);
    }

    projectModel.clearSelectedGeometry();
    return true;
}

bool ProjectModelLoader::loadMeshes(ProjectModel &projectModel, QString *errorMessage) const
{
    MeshManager meshManager(projectModel.project().rootPath);
    std::vector<MeshObject> loadedMeshes;
    if (!meshManager.loadMeshObjects(loadedMeshes, errorMessage)) {
        return false;
    }

    projectModel.meshObjects().clear();
    for (const MeshObject &meshObject : loadedMeshes) {
        projectModel.meshObjects().append(meshObject);
    }

    projectModel.clearSelectedMesh();
    return true;
}

bool ProjectModelLoader::loadSimulationCase(ProjectModel &projectModel, QString *errorMessage) const
{
    SimulationCaseManager simulationCaseManager;
    if (!simulationCaseManager.exists(projectModel.project())) {
        projectModel.materials().clear();
        projectModel.boundaryConditions().clear();
        projectModel.loads().clear();
        return true;
    }

    SimulationCase simulationCase;
    if (!simulationCaseManager.load(projectModel.project(), simulationCase, errorMessage)) {
        return false;
    }

    projectModel.materials() = simulationCase.materials;
    projectModel.boundaryConditions() = simulationCase.boundaryConditions;
    projectModel.loads() = simulationCase.loads;
    return true;
}
