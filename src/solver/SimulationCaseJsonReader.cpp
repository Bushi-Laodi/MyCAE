#include "solver/SimulationCaseJsonReader.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>

namespace
{
double numberValue(const QJsonObject &object, const QString &key, double defaultValue = 0.0)
{
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toDouble() : defaultValue;
}

bool boolValue(const QJsonObject &object, const QString &key, bool defaultValue = false)
{
    const QJsonValue value = object.value(key);
    return value.isBool() ? value.toBool() : defaultValue;
}

QString stringValue(const QJsonObject &object, const QString &key, const QString &defaultValue = {})
{
    const QJsonValue value = object.value(key);
    return value.isString() ? value.toString() : defaultValue;
}

MaterialDomain materialDomainFromString(const QString &value)
{
    return value.compare("solid", Qt::CaseInsensitive) == 0
        ? MaterialDomain::Solid
        : MaterialDomain::Fluid;
}

ViscosityModel viscosityModelFromString(const QString &value)
{
    Q_UNUSED(value);
    return ViscosityModel::Newtonian;
}

BoundaryTargetKind boundaryTargetKindFromString(const QString &value)
{
    return value.compare("meshBoundary", Qt::CaseInsensitive) == 0
        ? BoundaryTargetKind::MeshBoundary
        : BoundaryTargetKind::GeometryFaceGroup;
}

BoundaryConditionType boundaryConditionTypeFromString(const QString &value)
{
    if (value.compare("wall", Qt::CaseInsensitive) == 0) {
        return BoundaryConditionType::Wall;
    }
    if (value.compare("velocityInlet", Qt::CaseInsensitive) == 0) {
        return BoundaryConditionType::VelocityInlet;
    }
    if (value.compare("pressureInlet", Qt::CaseInsensitive) == 0) {
        return BoundaryConditionType::PressureInlet;
    }
    if (value.compare("pressureOutlet", Qt::CaseInsensitive) == 0) {
        return BoundaryConditionType::PressureOutlet;
    }
    if (value.compare("symmetry", Qt::CaseInsensitive) == 0) {
        return BoundaryConditionType::Symmetry;
    }
    return BoundaryConditionType::Unknown;
}

LoadType loadTypeFromString(const QString &value)
{
    if (value.compare("velocity", Qt::CaseInsensitive) == 0) {
        return LoadType::Velocity;
    }
    if (value.compare("pressure", Qt::CaseInsensitive) == 0) {
        return LoadType::Pressure;
    }
    if (value.compare("bodyForce", Qt::CaseInsensitive) == 0) {
        return LoadType::BodyForce;
    }
    return LoadType::Unknown;
}

LoadValueKind loadValueKindFromString(const QString &value)
{
    return value.compare("vector3", Qt::CaseInsensitive) == 0
        ? LoadValueKind::Vector3
        : LoadValueKind::Scalar;
}

FlowSolverType flowSolverTypeFromString(const QString &value)
{
    Q_UNUSED(value);
    return FlowSolverType::Simple;
}

TurbulenceModel turbulenceModelFromString(const QString &value)
{
    Q_UNUSED(value);
    return TurbulenceModel::KOmegaSST;
}

AxisDirection axisDirectionFromString(const QString &value)
{
    if (value.compare("X", Qt::CaseInsensitive) == 0) {
        return AxisDirection::X;
    }
    if (value.compare("Y", Qt::CaseInsensitive) == 0) {
        return AxisDirection::Y;
    }
    return AxisDirection::Z;
}

BooleanOperationType booleanOperationTypeFromString(const QString &value)
{
    Q_UNUSED(value);
    return BooleanOperationType::Union;
}

MeshElementType meshElementTypeFromString(const QString &value)
{
    if (value.compare("tetra10", Qt::CaseInsensitive) == 0) {
        return MeshElementType::Tetra10;
    }
    return MeshElementType::Tetra4;
}

Point3D pointFromJson(const QJsonObject &object)
{
    Point3D point;
    point.x = numberValue(object, "x");
    point.y = numberValue(object, "y");
    point.z = numberValue(object, "z");
    return point;
}

FacePoint facePointFromJson(const QJsonObject &object)
{
    FacePoint point;
    point.x = numberValue(object, "x");
    point.y = numberValue(object, "y");
    point.z = numberValue(object, "z");
    return point;
}

FaceReference faceReferenceFromJson(const QJsonObject &object)
{
    FaceReference reference;
    reference.faceIndex = object.value("faceIndex").toInt(-1);
    reference.pickedPoint = facePointFromJson(object.value("pickedPoint").toObject());
    reference.center = facePointFromJson(object.value("center").toObject());
    reference.normal = facePointFromJson(object.value("normal").toObject());
    reference.area = numberValue(object, "area");
    return reference;
}

CylinderDefinition cylinderFromJson(const QJsonObject &object)
{
    CylinderDefinition cylinder;
    cylinder.id = stringValue(object, "id");
    cylinder.name = stringValue(object, "name");
    cylinder.origin = pointFromJson(object.value("origin").toObject());
    cylinder.direction = axisDirectionFromString(stringValue(object, "direction"));
    cylinder.length = numberValue(object, "length");
    cylinder.radius = numberValue(object, "radius");
    return cylinder;
}

FaceGroup faceGroupFromJson(const QJsonObject &object)
{
    FaceGroup faceGroup;
    faceGroup.id = stringValue(object, "id");
    faceGroup.name = stringValue(object, "name");
    faceGroup.geometryName = stringValue(object, "geometryName");
    faceGroup.role = stringValue(object, "role");
    faceGroup.physicalGroupEnabled = boolValue(object, "physicalGroupEnabled", true);
    faceGroup.localMeshEnabled = boolValue(object, "localMeshEnabled");
    faceGroup.localMeshSize = numberValue(object, "localMeshSize");
    for (const QJsonValue &faceIndexValue : object.value("faceIndices").toArray()) {
        if (faceIndexValue.isDouble()) {
            faceGroup.faceIndices.push_back(faceIndexValue.toInt());
        }
    }
    for (const QJsonValue &referenceValue : object.value("faceReferences").toArray()) {
        faceGroup.faceReferences.push_back(faceReferenceFromJson(referenceValue.toObject()));
    }
    if (faceGroup.id.isEmpty() && !faceGroup.geometryName.isEmpty() && !faceGroup.name.isEmpty()) {
        faceGroup.id = FaceGroups::makeId(faceGroup.geometryName, faceGroup.name);
    }
    return faceGroup;
}

Material materialFromJson(const QJsonObject &object)
{
    Material material;
    material.id = stringValue(object, "id");
    material.name = stringValue(object, "name");
    material.domain = materialDomainFromString(stringValue(object, "domain"));
    material.viscosityModel = viscosityModelFromString(stringValue(object, "viscosityModel"));
    material.hasDensity = boolValue(object, "hasDensity");
    material.density = numberValue(object, "density");
    material.densityUnit = stringValue(object, "densityUnit", "kg/m^3");
    material.hasDynamicViscosity = boolValue(object, "hasDynamicViscosity");
    material.dynamicViscosity = numberValue(object, "dynamicViscosity");
    material.dynamicViscosityUnit = stringValue(object, "dynamicViscosityUnit", "Pa*s");
    material.hasKinematicViscosity = boolValue(object, "hasKinematicViscosity");
    material.kinematicViscosity = numberValue(object, "kinematicViscosity");
    material.kinematicViscosityUnit = stringValue(object, "kinematicViscosityUnit", "m^2/s");

    const QJsonArray extraProperties = object.value("extraProperties").toArray();
    for (const QJsonValue &propertyValue : extraProperties) {
        const QJsonObject propertyObject = propertyValue.toObject();
        MaterialProperty property;
        property.name = stringValue(propertyObject, "name");
        property.value = numberValue(propertyObject, "value");
        property.unit = stringValue(propertyObject, "unit");
        material.extraProperties.push_back(property);
    }
    return material;
}

BoundaryCondition boundaryConditionFromJson(const QJsonObject &object)
{
    const QJsonObject targetObject = object.value("target").toObject();

    BoundaryCondition boundaryCondition;
    boundaryCondition.id = stringValue(object, "id");
    boundaryCondition.name = stringValue(object, "name");
    boundaryCondition.type = boundaryConditionTypeFromString(stringValue(object, "type"));
    boundaryCondition.target.kind = boundaryTargetKindFromString(stringValue(targetObject, "kind"));
    boundaryCondition.target.geometryName = stringValue(targetObject, "geometryName");
    boundaryCondition.target.faceGroupId = stringValue(targetObject, "faceGroupId");
    boundaryCondition.target.faceGroupName = stringValue(targetObject, "faceGroupName");
    boundaryCondition.target.meshBoundaryName = stringValue(targetObject, "meshBoundaryName");
    boundaryCondition.materialId = stringValue(object, "materialId");
    boundaryCondition.enabled = boolValue(object, "enabled", true);
    return boundaryCondition;
}

Load loadFromJson(const QJsonObject &object)
{
    const QJsonObject valueObject = object.value("value").toObject();

    Load load;
    load.id = stringValue(object, "id");
    load.name = stringValue(object, "name");
    load.type = loadTypeFromString(stringValue(object, "type"));
    load.boundaryConditionId = stringValue(object, "boundaryConditionId");
    load.fieldName = stringValue(object, "fieldName");
    load.value.kind = loadValueKindFromString(stringValue(valueObject, "kind"));
    load.value.x = numberValue(valueObject, "x");
    load.value.y = numberValue(valueObject, "y");
    load.value.z = numberValue(valueObject, "z");
    load.value.unit = stringValue(valueObject, "unit");
    load.enabled = boolValue(object, "enabled", true);
    return load;
}

StructuralCase deriveStructuralCase(const SimulationCase &simulationCase)
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

CfdCase deriveCfdCase(const SimulationCase &simulationCase)
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

StructuralCase structuralCaseFromJson(const QJsonObject &object, const SimulationCase &fallback)
{
    if (object.isEmpty()) {
        return deriveStructuralCase(fallback);
    }

    StructuralCase structuralCase;
    structuralCase.id = stringValue(object, "id", fallback.id + "_structural");
    structuralCase.name = stringValue(object, "name", fallback.name + " - Structural");
    structuralCase.sourceGeometryName = stringValue(object, "sourceGeometryName", fallback.sourceGeometryName);
    structuralCase.meshName = stringValue(object, "meshName", fallback.meshName);
    for (const QJsonValue &materialValue : object.value("materials").toArray()) {
        structuralCase.materials.push_back(materialFromJson(materialValue.toObject()));
    }
    for (const QJsonValue &constraintValue : object.value("constraints").toArray()) {
        structuralCase.constraints.push_back(boundaryConditionFromJson(constraintValue.toObject()));
    }
    for (const QJsonValue &loadValue : object.value("loads").toArray()) {
        structuralCase.loads.push_back(loadFromJson(loadValue.toObject()));
    }
    return structuralCase;
}

CfdCase cfdCaseFromJson(const QJsonObject &object, const SimulationCase &fallback)
{
    if (object.isEmpty()) {
        return deriveCfdCase(fallback);
    }

    CfdCase cfdCase;
    cfdCase.id = stringValue(object, "id", fallback.id + "_cfd");
    cfdCase.name = stringValue(object, "name", fallback.name + " - CFD");
    cfdCase.sourceGeometryName = stringValue(object, "sourceGeometryName", fallback.sourceGeometryName);
    cfdCase.meshName = stringValue(object, "meshName", fallback.meshName);
    cfdCase.solverType = flowSolverTypeFromString(stringValue(object, "solverType"));
    cfdCase.turbulenceModel = turbulenceModelFromString(stringValue(object, "turbulenceModel"));

    const QJsonObject runControl = object.value("runControl").toObject();
    cfdCase.runControl.endTime = numberValue(runControl, "endTime", fallback.runControl.endTime);
    cfdCase.runControl.timeStep = numberValue(runControl, "timeStep", fallback.runControl.timeStep);
    cfdCase.runControl.writeInterval = numberValue(runControl, "writeInterval", fallback.runControl.writeInterval);
    cfdCase.runControl.cleanPreviousResult = boolValue(runControl, "cleanPreviousResult", fallback.runControl.cleanPreviousResult);

    for (const QJsonValue &materialValue : object.value("materials").toArray()) {
        cfdCase.materials.push_back(materialFromJson(materialValue.toObject()));
    }
    for (const QJsonValue &boundaryValue : object.value("boundaries").toArray()) {
        cfdCase.boundaries.push_back(boundaryConditionFromJson(boundaryValue.toObject()));
    }
    for (const QJsonValue &fieldValue : object.value("fieldValues").toArray()) {
        cfdCase.fieldValues.push_back(loadFromJson(fieldValue.toObject()));
    }
    return cfdCase;
}
}

bool SimulationCaseJsonReader::fromJson(
    const QJsonDocument &document,
    SimulationCase &simulationCase,
    QString *errorMessage
)
{
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "Simulation case JSON root is not an object.";
        }
        return false;
    }

    const QJsonObject root = document.object();
    SimulationCase loadedCase;
    loadedCase.id = stringValue(root, "id");
    loadedCase.name = stringValue(root, "name");
    loadedCase.sourceGeometryName = stringValue(root, "sourceGeometryName");
    loadedCase.meshName = stringValue(root, "meshName");
    loadedCase.solverType = flowSolverTypeFromString(stringValue(root, "solverType"));
    loadedCase.turbulenceModel = turbulenceModelFromString(stringValue(root, "turbulenceModel"));
    loadedCase.postProcessingTool = stringValue(root, "postProcessingTool");

    const QJsonObject geometrySetup = root.value("geometrySetup").toObject();
    for (const QJsonValue &cylinderValue : geometrySetup.value("cylinders").toArray()) {
        loadedCase.geometrySetup.cylinders.push_back(cylinderFromJson(cylinderValue.toObject()));
    }

    const QJsonObject booleanOperation = geometrySetup.value("booleanOperation").toObject();
    loadedCase.geometrySetup.booleanOperation.id = stringValue(booleanOperation, "id");
    loadedCase.geometrySetup.booleanOperation.type =
        booleanOperationTypeFromString(stringValue(booleanOperation, "type"));
    loadedCase.geometrySetup.booleanOperation.resultGeometryName =
        stringValue(booleanOperation, "resultGeometryName");
    for (const QJsonValue &geometryId : booleanOperation.value("inputGeometryIds").toArray()) {
        if (geometryId.isString()) {
            loadedCase.geometrySetup.booleanOperation.inputGeometryIds.push_back(geometryId.toString());
        }
    }

    for (const QJsonValue &faceGroupValue : geometrySetup.value("faceGroups").toArray()) {
        loadedCase.geometrySetup.faceGroups.push_back(faceGroupFromJson(faceGroupValue.toObject()));
    }

    const QJsonObject meshSetup = root.value("meshSetup").toObject();
    loadedCase.meshSetup.elementType = meshElementTypeFromString(stringValue(meshSetup, "elementType", "tetra4"));
    loadedCase.meshSetup.minimumSize = numberValue(meshSetup, "minimumSize");
    loadedCase.meshSetup.maximumSize = numberValue(meshSetup, "maximumSize", 1.0);
    loadedCase.meshSetup.autoSize = boolValue(meshSetup, "autoSize", true);
    loadedCase.meshSetup.localFaceGroupName = stringValue(meshSetup, "localFaceGroupName");
    loadedCase.meshSetup.autoImportAfterGeneration = boolValue(meshSetup, "autoImportAfterGeneration", true);
    loadedCase.meshSetup.showBoundaryAfterImport = boolValue(meshSetup, "showBoundaryAfterImport", true);

    const QJsonObject runControl = root.value("runControl").toObject();
    loadedCase.runControl.endTime = numberValue(runControl, "endTime", 400.0);
    loadedCase.runControl.timeStep = numberValue(runControl, "timeStep", 1.0);
    loadedCase.runControl.writeInterval = numberValue(runControl, "writeInterval", 100.0);
    loadedCase.runControl.cleanPreviousResult = boolValue(runControl, "cleanPreviousResult");

    for (const QJsonValue &materialValue : root.value("materials").toArray()) {
        loadedCase.materials.push_back(materialFromJson(materialValue.toObject()));
    }
    for (const QJsonValue &boundaryConditionValue : root.value("boundaryConditions").toArray()) {
        loadedCase.boundaryConditions.push_back(boundaryConditionFromJson(boundaryConditionValue.toObject()));
    }
    for (const QJsonValue &loadValue : root.value("loads").toArray()) {
        loadedCase.loads.push_back(loadFromJson(loadValue.toObject()));
    }

    loadedCase.structuralCase = structuralCaseFromJson(root.value("structuralCase").toObject(), loadedCase);
    loadedCase.cfdCase = cfdCaseFromJson(root.value("cfdCase").toObject(), loadedCase);

    simulationCase = loadedCase;
    return true;
}

bool SimulationCaseJsonReader::readCaseFile(
    const QString &filePath,
    SimulationCase &simulationCase,
    QString *errorMessage
)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "Failed to read simulation case file: " + file.errorString();
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (errorMessage) {
            *errorMessage = "Failed to parse simulation case JSON: " + parseError.errorString();
        }
        return false;
    }

    return fromJson(document, simulationCase, errorMessage);
}
