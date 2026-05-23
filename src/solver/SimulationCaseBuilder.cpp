#include "solver/SimulationCaseBuilder.h"

#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"
#include "project/ProjectModel.h"

#include <QSet>

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
    if (const GeometryObject *geometry = projectModel.selectedGeometry()) {
        return geometry->name;
    }
    if (const MeshObject *meshObject = projectModel.selectedMesh()) {
        return meshObject->sourceGeometryName;
    }
    if (!projectModel.geometryObjects().isEmpty()) {
        return projectModel.geometryObjects().front().name;
    }
    return {};
}

QString selectedOrFirstMeshName(const ProjectModel &projectModel)
{
    if (const MeshObject *meshObject = projectModel.selectedMesh()) {
        return meshObject->name;
    }
    if (!projectModel.meshObjects().isEmpty()) {
        return projectModel.meshObjects().front().name;
    }
    return {};
}

std::vector<FaceGroupDefinition> faceGroupsFromBoundaryConditions(
    const std::vector<BoundaryCondition> &boundaryConditions
)
{
    QSet<QString> seen;
    std::vector<FaceGroupDefinition> faceGroups;
    for (const BoundaryCondition &boundaryCondition : boundaryConditions) {
        const QString faceGroupName = boundaryCondition.target.faceGroupName.trimmed();
        if (faceGroupName.isEmpty() || seen.contains(faceGroupName)) {
            continue;
        }

        seen.insert(faceGroupName);
        FaceGroupDefinition faceGroup;
        faceGroup.name = faceGroupName;
        faceGroup.role = toString(boundaryCondition.type);
        faceGroups.push_back(faceGroup);
    }
    return faceGroups;
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
    simulationCase.materials = projectModel.materials();
    simulationCase.boundaryConditions = projectModel.boundaryConditions();
    simulationCase.loads = projectModel.loads();
    simulationCase.geometrySetup.faceGroups = faceGroupsFromBoundaryConditions(simulationCase.boundaryConditions);
    return simulationCase;
}
