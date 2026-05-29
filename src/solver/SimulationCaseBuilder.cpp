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

StructuralCase buildStructuralCase(const SimulationCase &simulationCase)
{
    StructuralCase structuralCase;
    structuralCase.id = simulationCase.id + "_structural";
    structuralCase.name = simulationCase.name + " - Structural";
    structuralCase.sourceGeometryName = simulationCase.sourceGeometryName;
    structuralCase.meshName = simulationCase.meshName;

    for (const Material &material : simulationCase.materials) {
        if (isStructuralMaterial(material)) {
            structuralCase.materials.push_back(material);
        }
    }
    for (const SectionAssignment &sectionAssignment : simulationCase.sectionAssignments) {
        if (sectionAssignment.enabled) {
            structuralCase.sectionAssignments.push_back(sectionAssignment);
        }
    }
    if (structuralCase.sectionAssignments.empty() && !structuralCase.materials.empty()) {
        SectionAssignment sectionAssignment;
        sectionAssignment.id = simulationCase.id + "_default_section";
        sectionAssignment.name = "Default Solid Section";
        sectionAssignment.materialId = structuralCase.materials.front().id;
        sectionAssignment.geometryName = simulationCase.sourceGeometryName;
        sectionAssignment.meshName = simulationCase.meshName;
        sectionAssignment.elementSetName = "EALL";
        structuralCase.sectionAssignments.push_back(sectionAssignment);
    }
    for (const BoundaryCondition &boundaryCondition : simulationCase.boundaryConditions) {
        if (isStructuralConstraint(boundaryCondition, simulationCase.loads)) {
            structuralCase.constraints.push_back(boundaryCondition);
        }
    }
    for (const Load &load : simulationCase.loads) {
        if (load.enabled && isStructuralLoadType(load.type)) {
            structuralCase.loads.push_back(load);
        }
    }
    return structuralCase;
}

CfdCase buildCfdCase(const SimulationCase &simulationCase)
{
    CfdCase cfdCase;
    cfdCase.id = simulationCase.id + "_cfd";
    cfdCase.name = simulationCase.name + " - CFD";
    cfdCase.sourceGeometryName = simulationCase.sourceGeometryName;
    cfdCase.meshName = simulationCase.meshName;
    cfdCase.solverType = simulationCase.solverType;
    cfdCase.turbulenceModel = simulationCase.turbulenceModel;
    cfdCase.runControl = simulationCase.runControl;

    for (const Material &material : simulationCase.materials) {
        if (isCfdMaterial(material)) {
            cfdCase.materials.push_back(material);
        }
    }
    for (const BoundaryCondition &boundaryCondition : simulationCase.boundaryConditions) {
        if (isCfdBoundary(boundaryCondition)) {
            cfdCase.boundaries.push_back(boundaryCondition);
        }
    }
    for (const Load &load : simulationCase.loads) {
        if (load.enabled && isCfdFieldValueType(load.type)) {
            cfdCase.fieldValues.push_back(load);
        }
    }
    return cfdCase;
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
    simulationCase.meshSetup = projectModel.meshRepository().meshSetup();
    simulationCase.postProcessingTool = "ParaView";
    const SolverRepository &solverRepository = projectModel.solverRepository();
    simulationCase.materials = solverRepository.materials();
    simulationCase.sectionAssignments = solverRepository.sectionAssignments();
    simulationCase.boundaryConditions = solverRepository.boundaryConditions();
    simulationCase.loads = solverRepository.loads();
    simulationCase.geometrySetup.faceGroups = solverRepository.faceGroups();
    simulationCase.structuralCase = buildStructuralCase(simulationCase);
    simulationCase.cfdCase = buildCfdCase(simulationCase);
    return simulationCase;
}
