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

    GeometryRepository &geometryRepository = projectModel.geometryRepository();
    geometryRepository.boxes() = loadedBoxes;
    geometryRepository.cylinders() = loadedCylinders;
    geometryRepository.geometryObjects().clear();
    for (const GeometryObject &geometry : loadedGeometries) {
        geometryRepository.geometryObjects().append(geometry);
    }
    projectModel.ensureDefaultFaceGroups();

    projectModel.clearSelectionIfKind(SelectionKind::Geometry);
    return true;
}

bool ProjectModelLoader::loadMeshes(ProjectModel &projectModel, QString *errorMessage) const
{
    MeshManager meshManager(projectModel.project().rootPath);
    std::vector<MeshObject> loadedMeshes;
    if (!meshManager.loadMeshObjects(loadedMeshes, errorMessage)) {
        return false;
    }

    MeshRepository &meshRepository = projectModel.meshRepository();
    meshRepository.meshObjects().clear();
    for (const MeshObject &meshObject : loadedMeshes) {
        meshRepository.meshObjects().append(meshObject);
    }

    projectModel.clearSelectionIfKind(SelectionKind::Mesh);
    return true;
}

bool ProjectModelLoader::loadSimulationCase(ProjectModel &projectModel, QString *errorMessage) const
{
    SimulationCaseManager simulationCaseManager;
    if (!simulationCaseManager.exists(projectModel.project())) {
        projectModel.solverRepository().clearSolverData();
        projectModel.ensureDefaultFaceGroups();
        return true;
    }

    SimulationCase simulationCase;
    if (!simulationCaseManager.load(projectModel.project(), simulationCase, errorMessage)) {
        return false;
    }

    SolverRepository &solverRepository = projectModel.solverRepository();
    solverRepository.materials() = simulationCase.materials;
    solverRepository.boundaryConditions() = simulationCase.boundaryConditions;
    solverRepository.loads() = simulationCase.loads;
    solverRepository.faceGroups() = simulationCase.geometrySetup.faceGroups;
    projectModel.ensureDefaultFaceGroups();
    return true;
}
