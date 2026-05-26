#include "solver/calculix/CalculiXCaseDataBuilder.h"

#include "mesh/MeshObject.h"
#include "mesh/MshReader.h"
#include "project/ProjectModel.h"
#include "solver/SimulationCase.h"
#include "solver/calculix/CalculiXMeshBoundaryResolver.h"

#include <QDir>
#include <QFileInfo>

namespace
{
QString normalizedPropertyName(QString name)
{
    name = name.trimmed().toLower();
    QString normalized;
    normalized.reserve(name.size());
    for (const QChar ch : name) {
        if (ch.isLetterOrNumber()) {
            normalized.append(ch);
        }
    }
    return normalized;
}

bool propertyMatches(const QString &normalizedName, const QStringList &aliases)
{
    for (const QString &alias : aliases) {
        if (normalizedName == normalizedPropertyName(alias)) {
            return true;
        }
    }
    return false;
}

double materialPropertyValue(const Material &material, const QStringList &aliases, double fallback = 0.0)
{
    for (const MaterialProperty &property : material.extraProperties) {
        if (propertyMatches(normalizedPropertyName(property.name), aliases)) {
            return property.value;
        }
    }
    return fallback;
}

CalculiXMaterialData toCalculiXMaterial(const Material &material)
{
    CalculiXMaterialData materialData;
    materialData.id = material.id;
    materialData.name = material.name;
    materialData.domain = material.domain;
    materialData.youngModulus = materialPropertyValue(
        material,
        {"youngModulus", "youngsModulus", "elasticModulus", "E"}
    );
    materialData.poissonRatio = materialPropertyValue(
        material,
        {"poissonRatio", "poissonsRatio", "nu"}
    );
    materialData.density = material.hasDensity
        ? material.density
        : materialPropertyValue(material, {"density", "rho"});
    return materialData;
}

CalculiXBoundaryData toCalculiXBoundary(const BoundaryCondition &boundaryCondition)
{
    CalculiXBoundaryData boundaryData;
    boundaryData.id = boundaryCondition.id;
    boundaryData.name = boundaryCondition.name;
    boundaryData.type = boundaryCondition.type;
    boundaryData.materialId = boundaryCondition.materialId;
    boundaryData.geometryName = boundaryCondition.target.geometryName;
    boundaryData.faceGroupId = boundaryCondition.target.faceGroupId;
    boundaryData.faceGroupName = boundaryCondition.target.faceGroupName;
    boundaryData.meshBoundaryName = boundaryCondition.target.meshBoundaryName;
    return boundaryData;
}

CalculiXLoadData toCalculiXLoad(const Load &load)
{
    CalculiXLoadData loadData;
    loadData.id = load.id;
    loadData.name = load.name;
    loadData.type = load.type;
    loadData.boundaryConditionId = load.boundaryConditionId;
    loadData.fieldName = load.fieldName;
    loadData.value = load.value;
    return loadData;
}

QString absoluteProjectPath(const ProjectModel &projectModel, const QString &path)
{
    return QFileInfo(path).isAbsolute()
        ? path
        : QDir(projectModel.project().rootPath).filePath(path);
}

void validateMaterial(const CalculiXMaterialData &material, CalculiXCaseDataBuildResult &result)
{
    if (material.domain != MaterialDomain::Solid) {
        result.errors.append("CalculiX material must be solid: " + material.name);
    }
    if (material.youngModulus <= 0.0) {
        result.errors.append("CalculiX material is missing positive youngModulus: " + material.name);
    }
    if (material.poissonRatio < 0.0 || material.poissonRatio >= 0.5) {
        result.errors.append("CalculiX material has invalid poissonRatio: " + material.name);
    }
}

}

CalculiXCaseDataBuildResult CalculiXCaseDataBuilder::build(const SolverCaseContext &context) const
{
    CalculiXCaseDataBuildResult result;
    if (!context.isValid()) {
        result.errors.append("CalculiX case data build failed: invalid solver case context.");
        return result;
    }

    const ProjectModel &projectModel = *context.projectModel;
    const SimulationCase &simulationCase = *context.simulationCase;
    result.caseData.caseName = simulationCase.name;

    if (simulationCase.meshName.trimmed().isEmpty()) {
        result.errors.append("CalculiX case data build failed: simulation case has no mesh.");
        return result;
    }

    const MeshObject *meshObject = projectModel.meshRepository().findMeshByName(simulationCase.meshName);
    if (!meshObject) {
        result.errors.append("CalculiX case data build failed: mesh not found: " + simulationCase.meshName);
        return result;
    }

    const QString meshPath = absoluteProjectPath(projectModel, meshObject->mshFile);
    if (!QFileInfo::exists(meshPath)) {
        result.errors.append("CalculiX case data build failed: mesh file does not exist: " + meshPath);
        return result;
    }

    QString meshReadError;
    MeshData meshData;
    meshData.name = meshObject->name;
    meshData.sourceGeometryName = meshObject->sourceGeometryName;
    if (!MshReader::readMsh2(meshPath, meshData, &meshReadError)) {
        result.errors.append("CalculiX case data build failed: cannot read MSH: " + meshReadError);
        return result;
    }
    if (meshData.nodes.empty() || meshData.tetraCount() == 0) {
        result.errors.append("CalculiX case data build failed: mesh has no tetrahedral elements.");
        return result;
    }

    result.caseData.meshName = meshObject->name;
    result.caseData.meshFile = meshPath;
    result.caseData.meshData = meshData;

    for (const MeshBoundary &meshBoundary : projectModel.meshRepository().meshBoundaries()) {
        if (meshBoundary.meshName == meshObject->name) {
            result.caseData.meshBoundaries.push_back(meshBoundary);
        }
    }

    for (const Material &material : simulationCase.materials) {
        CalculiXMaterialData materialData = toCalculiXMaterial(material);
        validateMaterial(materialData, result);
        result.caseData.materials.push_back(materialData);
    }
    if (result.caseData.materials.empty()) {
        result.errors.append("CalculiX case data build failed: no material is defined.");
    }

    for (const BoundaryCondition &boundaryCondition : simulationCase.boundaryConditions) {
        if (boundaryCondition.enabled) {
            result.caseData.boundaries.push_back(toCalculiXBoundary(boundaryCondition));
        }
    }
    if (result.caseData.boundaries.empty()) {
        result.warnings.append("CalculiX case has no enabled boundary conditions.");
    }

    for (const Load &load : simulationCase.loads) {
        if (load.enabled) {
            result.caseData.loads.push_back(toCalculiXLoad(load));
        }
    }
    if (result.caseData.loads.empty()) {
        result.warnings.append("CalculiX case has no enabled loads.");
    }

    const CalculiXMeshBoundaryResolveResult boundaryResolveResult =
        CalculiXMeshBoundaryResolver().resolve(projectModel, result.caseData);
    result.caseData.meshBoundaries = boundaryResolveResult.meshBoundaries;
    result.warnings.append(boundaryResolveResult.warnings);
    result.logMessages.append(boundaryResolveResult.logMessages);

    result.success = result.errors.empty();
    result.logMessages.append(QString("CalculiX case data: %1 nodes, %2 tetrahedra, %3 material(s), %4 boundary(ies), %5 load(s).")
        .arg(result.caseData.meshData.nodeCount())
        .arg(result.caseData.meshData.tetraCount())
        .arg(static_cast<int>(result.caseData.materials.size()))
        .arg(static_cast<int>(result.caseData.boundaries.size()))
        .arg(static_cast<int>(result.caseData.loads.size())));
    return result;
}
