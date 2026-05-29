#include "solver/SimulationCaseJsonWriter.h"

#include "solver/SimulationCase.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>

namespace
{
QJsonObject pointToJson(const Point3D &point)
{
    QJsonObject object;
    object.insert("x", point.x);
    object.insert("y", point.y);
    object.insert("z", point.z);
    return object;
}

QJsonObject facePointToJson(const FacePoint &point)
{
    QJsonObject object;
    object.insert("x", point.x);
    object.insert("y", point.y);
    object.insert("z", point.z);
    return object;
}

QJsonObject faceReferenceToJson(const FaceReference &reference)
{
    QJsonObject object;
    object.insert("faceIndex", reference.faceIndex);
    object.insert("pickedPoint", facePointToJson(reference.pickedPoint));
    object.insert("center", facePointToJson(reference.center));
    object.insert("normal", facePointToJson(reference.normal));
    object.insert("area", reference.area);
    return object;
}

QJsonObject cylinderToJson(const CylinderDefinition &cylinder)
{
    QJsonObject object;
    object.insert("id", cylinder.id);
    object.insert("name", cylinder.name);
    object.insert("origin", pointToJson(cylinder.origin));
    object.insert("direction", toString(cylinder.direction));
    object.insert("length", cylinder.length);
    object.insert("radius", cylinder.radius);
    return object;
}

QJsonObject faceGroupToJson(const FaceGroup &faceGroup)
{
    QJsonArray faceIndices;
    for (const int faceIndex : faceGroup.faceIndices) {
        faceIndices.append(faceIndex);
    }
    QJsonArray faceReferences;
    for (const FaceReference &reference : faceGroup.faceReferences) {
        faceReferences.append(faceReferenceToJson(reference));
    }

    QJsonObject object;
    object.insert("id", faceGroup.id);
    object.insert("name", faceGroup.name);
    object.insert("geometryName", faceGroup.geometryName);
    object.insert("role", faceGroup.role);
    object.insert("faceIndices", faceIndices);
    object.insert("faceReferences", faceReferences);
    object.insert("physicalGroupEnabled", faceGroup.physicalGroupEnabled);
    object.insert("localMeshEnabled", faceGroup.localMeshEnabled);
    object.insert("localMeshSize", faceGroup.localMeshSize);
    object.insert("needsReview", faceGroup.needsReview);
    object.insert("reviewReason", faceGroup.reviewReason);
    return object;
}

QJsonObject materialToJson(const Material &material)
{
    QJsonObject object;
    object.insert("id", material.id);
    object.insert("name", material.name);
    object.insert("domain", toString(material.domain));
    object.insert("viscosityModel", toString(material.viscosityModel));
    object.insert("hasDensity", material.hasDensity);
    object.insert("density", material.density);
    object.insert("densityUnit", material.densityUnit);
    object.insert("hasDynamicViscosity", material.hasDynamicViscosity);
    object.insert("dynamicViscosity", material.dynamicViscosity);
    object.insert("dynamicViscosityUnit", material.dynamicViscosityUnit);
    object.insert("hasKinematicViscosity", material.hasKinematicViscosity);
    object.insert("kinematicViscosity", material.kinematicViscosity);
    object.insert("kinematicViscosityUnit", material.kinematicViscosityUnit);

    QJsonArray extraProperties;
    for (const MaterialProperty &property : material.extraProperties) {
        QJsonObject propertyObject;
        propertyObject.insert("name", property.name);
        propertyObject.insert("value", property.value);
        propertyObject.insert("unit", property.unit);
        extraProperties.append(propertyObject);
    }
    object.insert("extraProperties", extraProperties);
    return object;
}

QJsonObject sectionAssignmentToJson(const SectionAssignment &sectionAssignment)
{
    QJsonObject object;
    object.insert("id", sectionAssignment.id);
    object.insert("name", sectionAssignment.name);
    object.insert("materialId", sectionAssignment.materialId);
    object.insert("geometryName", sectionAssignment.geometryName);
    object.insert("meshName", sectionAssignment.meshName);
    object.insert("elementSetName", sectionAssignment.elementSetName);
    object.insert("enabled", sectionAssignment.enabled);
    return object;
}

QJsonObject boundaryConditionToJson(const BoundaryCondition &boundaryCondition)
{
    QJsonObject target;
    target.insert("kind", toString(boundaryCondition.target.kind));
    target.insert("geometryName", boundaryCondition.target.geometryName);
    target.insert("faceGroupId", boundaryCondition.target.faceGroupId);
    target.insert("faceGroupName", boundaryCondition.target.faceGroupName);
    target.insert("meshBoundaryName", boundaryCondition.target.meshBoundaryName);

    QJsonObject object;
    object.insert("id", boundaryCondition.id);
    object.insert("name", boundaryCondition.name);
    object.insert("type", toString(boundaryCondition.type));
    object.insert("target", target);
    object.insert("materialId", boundaryCondition.materialId);
    object.insert("enabled", boundaryCondition.enabled);
    return object;
}

QJsonObject loadToJson(const Load &load)
{
    QJsonObject value;
    value.insert("kind", toString(load.value.kind));
    value.insert("x", load.value.x);
    value.insert("y", load.value.y);
    value.insert("z", load.value.z);
    value.insert("unit", load.value.unit);

    QJsonObject object;
    object.insert("id", load.id);
    object.insert("name", load.name);
    object.insert("type", toString(load.type));
    object.insert("boundaryConditionId", load.boundaryConditionId);
    object.insert("fieldName", load.fieldName);
    object.insert("value", value);
    object.insert("enabled", load.enabled);
    return object;
}

QJsonObject structuralCaseToJson(const StructuralCase &structuralCase)
{
    QJsonArray materials;
    for (const Material &material : structuralCase.materials) {
        materials.append(materialToJson(material));
    }
    QJsonArray constraints;
    for (const BoundaryCondition &constraint : structuralCase.constraints) {
        constraints.append(boundaryConditionToJson(constraint));
    }
    QJsonArray sectionAssignments;
    for (const SectionAssignment &sectionAssignment : structuralCase.sectionAssignments) {
        sectionAssignments.append(sectionAssignmentToJson(sectionAssignment));
    }
    QJsonArray loads;
    for (const Load &load : structuralCase.loads) {
        loads.append(loadToJson(load));
    }

    QJsonObject object;
    object.insert("id", structuralCase.id);
    object.insert("name", structuralCase.name);
    object.insert("sourceGeometryName", structuralCase.sourceGeometryName);
    object.insert("meshName", structuralCase.meshName);
    object.insert("materials", materials);
    object.insert("sectionAssignments", sectionAssignments);
    object.insert("constraints", constraints);
    object.insert("loads", loads);
    return object;
}

QJsonObject cfdCaseToJson(const CfdCase &cfdCase)
{
    QJsonArray materials;
    for (const Material &material : cfdCase.materials) {
        materials.append(materialToJson(material));
    }
    QJsonArray boundaries;
    for (const BoundaryCondition &boundary : cfdCase.boundaries) {
        boundaries.append(boundaryConditionToJson(boundary));
    }
    QJsonArray fieldValues;
    for (const Load &fieldValue : cfdCase.fieldValues) {
        fieldValues.append(loadToJson(fieldValue));
    }

    QJsonObject runControl;
    runControl.insert("endTime", cfdCase.runControl.endTime);
    runControl.insert("timeStep", cfdCase.runControl.timeStep);
    runControl.insert("writeInterval", cfdCase.runControl.writeInterval);
    runControl.insert("cleanPreviousResult", cfdCase.runControl.cleanPreviousResult);

    QJsonObject object;
    object.insert("id", cfdCase.id);
    object.insert("name", cfdCase.name);
    object.insert("sourceGeometryName", cfdCase.sourceGeometryName);
    object.insert("meshName", cfdCase.meshName);
    object.insert("solverType", toString(cfdCase.solverType));
    object.insert("turbulenceModel", toString(cfdCase.turbulenceModel));
    object.insert("runControl", runControl);
    object.insert("materials", materials);
    object.insert("boundaries", boundaries);
    object.insert("fieldValues", fieldValues);
    return object;
}
}

QJsonDocument SimulationCaseJsonWriter::toJson(const SimulationCase &simulationCase)
{
    QJsonArray cylinders;
    for (const CylinderDefinition &cylinder : simulationCase.geometrySetup.cylinders) {
        cylinders.append(cylinderToJson(cylinder));
    }

    QJsonArray booleanInputs;
    for (const QString &geometryId : simulationCase.geometrySetup.booleanOperation.inputGeometryIds) {
        booleanInputs.append(geometryId);
    }

    QJsonObject booleanOperation;
    booleanOperation.insert("id", simulationCase.geometrySetup.booleanOperation.id);
    booleanOperation.insert("type", toString(simulationCase.geometrySetup.booleanOperation.type));
    booleanOperation.insert("inputGeometryIds", booleanInputs);
    booleanOperation.insert("resultGeometryName", simulationCase.geometrySetup.booleanOperation.resultGeometryName);

    QJsonArray faceGroups;
    for (const FaceGroup &faceGroup : simulationCase.geometrySetup.faceGroups) {
        faceGroups.append(faceGroupToJson(faceGroup));
    }

    QJsonObject geometrySetup;
    geometrySetup.insert("cylinders", cylinders);
    geometrySetup.insert("booleanOperation", booleanOperation);
    geometrySetup.insert("faceGroups", faceGroups);

    QJsonObject meshSetup;
    meshSetup.insert("elementType", toString(simulationCase.meshSetup.elementType));
    meshSetup.insert("minimumSize", simulationCase.meshSetup.minimumSize);
    meshSetup.insert("maximumSize", simulationCase.meshSetup.maximumSize);
    meshSetup.insert("autoSize", simulationCase.meshSetup.autoSize);
    meshSetup.insert("localFaceGroupName", simulationCase.meshSetup.localFaceGroupName);
    meshSetup.insert("autoImportAfterGeneration", simulationCase.meshSetup.autoImportAfterGeneration);
    meshSetup.insert("showBoundaryAfterImport", simulationCase.meshSetup.showBoundaryAfterImport);

    QJsonObject runControl;
    runControl.insert("endTime", simulationCase.runControl.endTime);
    runControl.insert("timeStep", simulationCase.runControl.timeStep);
    runControl.insert("writeInterval", simulationCase.runControl.writeInterval);
    runControl.insert("cleanPreviousResult", simulationCase.runControl.cleanPreviousResult);

    QJsonArray materials;
    for (const Material &material : simulationCase.materials) {
        materials.append(materialToJson(material));
    }

    QJsonArray sectionAssignments;
    for (const SectionAssignment &sectionAssignment : simulationCase.sectionAssignments) {
        sectionAssignments.append(sectionAssignmentToJson(sectionAssignment));
    }

    QJsonArray boundaryConditions;
    for (const BoundaryCondition &boundaryCondition : simulationCase.boundaryConditions) {
        boundaryConditions.append(boundaryConditionToJson(boundaryCondition));
    }

    QJsonArray loads;
    for (const Load &load : simulationCase.loads) {
        loads.append(loadToJson(load));
    }

    QJsonObject root;
    root.insert("id", simulationCase.id);
    root.insert("name", simulationCase.name);
    root.insert("sourceGeometryName", simulationCase.sourceGeometryName);
    root.insert("meshName", simulationCase.meshName);
    root.insert("geometrySetup", geometrySetup);
    root.insert("meshSetup", meshSetup);
    root.insert("solverType", toString(simulationCase.solverType));
    root.insert("turbulenceModel", toString(simulationCase.turbulenceModel));
    root.insert("runControl", runControl);
    root.insert("postProcessingTool", simulationCase.postProcessingTool);
    root.insert("materials", materials);
    root.insert("sectionAssignments", sectionAssignments);
    root.insert("boundaryConditions", boundaryConditions);
    root.insert("loads", loads);
    root.insert("structuralCase", structuralCaseToJson(simulationCase.structuralCase));
    root.insert("cfdCase", cfdCaseToJson(simulationCase.cfdCase));
    return QJsonDocument(root);
}

bool SimulationCaseJsonWriter::writeCaseFile(
    const SimulationCase &simulationCase,
    const QString &filePath,
    QString *errorMessage
)
{
    const QFileInfo fileInfo(filePath);
    if (!QDir().mkpath(fileInfo.absolutePath())) {
        if (errorMessage) {
            *errorMessage = "Failed to create directory: " + fileInfo.absolutePath();
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write case file: " + file.errorString();
        }
        return false;
    }

    file.write(toJson(simulationCase).toJson(QJsonDocument::Indented));
    return true;
}
