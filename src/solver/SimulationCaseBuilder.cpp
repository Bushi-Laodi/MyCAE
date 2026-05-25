#include "solver/SimulationCaseBuilder.h"

#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"
#include "project/ProjectModel.h"

namespace
{
QString makeId(QString value)
{
    value = value.trimmed().toLower();
    for (QChar &ch : value) {
        if (ch.isSpace()) {
            ch = '_';
        } else if (!ch.isLetterOrNumber() && ch != '_') {
            ch = '_';
        }
    }
    return value.isEmpty() ? QString("simulation_case") : value;
}

QString selectedOrFirstGeometryName(const ProjectModel &projectModel)
{
    if (const GeometryObject *geometry = projectModel.geometryForSelection()) {
        return geometry->name;
    }
    if (const MeshObject *meshObject = projectModel.meshForSelection()) {
        return meshObject->sourceGeometryName;
    }
    const GeometryRepository &geometryRepository = projectModel.geometryRepository();
    if (!geometryRepository.geometryObjects().isEmpty()) {
        return geometryRepository.geometryObjects().front().name;
    }
    return {};
}

QString selectedOrFirstMeshName(const ProjectModel &projectModel)
{
    if (const MeshObject *meshObject = projectModel.meshForSelection()) {
        return meshObject->name;
    }
    const MeshRepository &meshRepository = projectModel.meshRepository();
    if (!meshRepository.meshObjects().isEmpty()) {
        return meshRepository.meshObjects().front().name;
    }
    return {};
}

}

SimulationCase SimulationCaseBuilder::fromProjectModel(const ProjectModel &projectModel)
{
    SimulationCase simulationCase;
    simulationCase.id = makeId(projectModel.project().name);
    simulationCase.name = projectModel.project().name.isEmpty()
        ? QString("MyCAE Simulation Case")
        : projectModel.project().name + " Simulation Case";
    simulationCase.sourceGeometryName = selectedOrFirstGeometryName(projectModel);
    simulationCase.meshName = selectedOrFirstMeshName(projectModel);
    simulationCase.postProcessingTool = "ParaView";
    const SolverRepository &solverRepository = projectModel.solverRepository();
    simulationCase.materials = solverRepository.materials();
    simulationCase.boundaryConditions = solverRepository.boundaryConditions();
    simulationCase.loads = solverRepository.loads();
    simulationCase.geometrySetup.faceGroups = solverRepository.faceGroups();
    return simulationCase;
}
