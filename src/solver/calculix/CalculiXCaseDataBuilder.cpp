#include "solver/calculix/CalculiXCaseDataBuilder.h"

#include "mesh/MeshObject.h"
#include "mesh/MshReader.h"
#include "project/ProjectModel.h"
#include "solver/SimulationCase.h"
#include "solver/calculix/CalculiXMeshBoundaryResolver.h"

#include <QDir>
#include <QFileInfo>

#include <algorithm>

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

const CalculiXMaterialData *findMaterialById(
    const std::vector<CalculiXMaterialData> &materials,
    const QString &materialId
)
{
    for (const CalculiXMaterialData &material : materials) {
        if (material.id == materialId) {
            return &material;
        }
    }
    return nullptr;
}

std::vector<int> allElementIds(const MeshData &meshData)
{
    std::vector<int> elementIds;
    elementIds.reserve(meshData.tetraElements.size() + meshData.tetra10Elements.size());
    for (const TetraElement &tetra : meshData.tetraElements) {
        elementIds.push_back(tetra.id);
    }
    for (const Tetra10Element &tetra : meshData.tetra10Elements) {
        elementIds.push_back(tetra.id);
    }
    std::sort(elementIds.begin(), elementIds.end());
    return elementIds;
}

bool isAllElementSetName(const QString &elementSetName)
{
    const QString value = elementSetName.trimmed();
    return value.isEmpty() || value.compare("EALL", Qt::CaseInsensitive) == 0;
}

int volumePhysicalGroupTagForElementSet(const MeshData &meshData, const QString &elementSetName)
{
    const QString target = elementSetName.trimmed();
    if (target.isEmpty()) {
        return -1;
    }

    bool targetIsNumber = false;
    const int targetTag = target.toInt(&targetIsNumber);
    for (const MeshPhysicalGroup &physicalGroup : meshData.physicalGroups) {
        if (physicalGroup.dimension != 3) {
            continue;
        }
        if ((targetIsNumber && physicalGroup.tag == targetTag)
                || physicalGroup.name.compare(target, Qt::CaseInsensitive) == 0) {
            return physicalGroup.tag;
        }
    }
    return targetIsNumber ? targetTag : -1;
}

std::vector<int> elementIdsForPhysicalGroup(const MeshData &meshData, int physicalGroupTag)
{
    std::vector<int> elementIds;
    for (const TetraElement &tetra : meshData.tetraElements) {
        if (tetra.physicalGroupTag == physicalGroupTag) {
            elementIds.push_back(tetra.id);
        }
    }
    for (const Tetra10Element &tetra : meshData.tetra10Elements) {
        if (tetra.physicalGroupTag == physicalGroupTag) {
            elementIds.push_back(tetra.id);
        }
    }
    std::sort(elementIds.begin(), elementIds.end());
    return elementIds;
}

bool sectionTargetsCurrentMesh(
    const SectionAssignment &sectionAssignment,
    const MeshObject &meshObject,
    const MeshData &meshData
)
{
    if (!sectionAssignment.meshName.trimmed().isEmpty()
            && sectionAssignment.meshName != meshObject.name
            && sectionAssignment.meshName != meshData.name) {
        return false;
    }
    if (!sectionAssignment.geometryName.trimmed().isEmpty()
            && !meshObject.sourceGeometryName.trimmed().isEmpty()
            && sectionAssignment.geometryName != meshObject.sourceGeometryName) {
        return false;
    }
    return true;
}

void buildSectionAssignments(
    const StructuralCase &structuralCase,
    const MeshObject &meshObject,
    CalculiXCaseDataBuildResult &result
)
{
    const MeshData &meshData = result.caseData.meshData;
    const std::vector<int> allIds = allElementIds(meshData);
    std::vector<int> usedElementIds;
    int exportedSectionIndex = 0;
    bool hasFullMeshSection = false;
    bool hadEnabledSectionAssignment = false;

    for (const SectionAssignment &sectionAssignment : structuralCase.sectionAssignments) {
        if (!sectionAssignment.enabled) {
            continue;
        }
        hadEnabledSectionAssignment = true;
        if (!sectionTargetsCurrentMesh(sectionAssignment, meshObject, meshData)) {
            result.warnings.append("CalculiX section assignment skipped because it targets another mesh or geometry: "
                + sectionAssignment.name);
            continue;
        }

        const CalculiXMaterialData *material = findMaterialById(
            result.caseData.materials,
            sectionAssignment.materialId
        );
        if (!material) {
            result.errors.append("CalculiX section assignment references missing material: "
                + sectionAssignment.name);
            continue;
        }

        CalculiXSectionAssignmentData sectionData;
        sectionData.id = sectionAssignment.id;
        sectionData.name = sectionAssignment.name;
        sectionData.materialId = material->id;
        sectionData.materialName = material->id.trimmed().isEmpty() ? material->name : material->id;
        sectionData.geometryName = sectionAssignment.geometryName;
        sectionData.meshName = sectionAssignment.meshName;
        sectionData.enabled = sectionAssignment.enabled;

        if (isAllElementSetName(sectionAssignment.elementSetName)) {
            if (exportedSectionIndex > 0) {
                result.errors.append("CalculiX section assignment would overlap the full mesh; set a volume physical group elementSetName: "
                    + sectionAssignment.name);
                continue;
            }
            hasFullMeshSection = true;
            sectionData.elementSetName = "EALL";
            sectionData.elementIds = allIds;
        } else {
            if (hasFullMeshSection) {
                result.errors.append("CalculiX section assignment overlaps an existing full-mesh section: "
                    + sectionAssignment.name);
                continue;
            }
            sectionData.elementSetName = sectionAssignment.elementSetName.trimmed();
            const int physicalGroupTag =
                volumePhysicalGroupTagForElementSet(meshData, sectionAssignment.elementSetName);
            sectionData.elementIds = elementIdsForPhysicalGroup(meshData, physicalGroupTag);
            if (sectionData.elementIds.empty()) {
                result.errors.append("CalculiX section assignment has no matching volume elements: "
                    + sectionAssignment.name
                    + " / elementSetName=" + sectionAssignment.elementSetName);
                continue;
            }
        }

        bool overlapsExistingSection = false;
        for (const int elementId : sectionData.elementIds) {
            if (std::binary_search(usedElementIds.begin(), usedElementIds.end(), elementId)) {
                overlapsExistingSection = true;
                break;
            }
        }
        if (overlapsExistingSection) {
            result.errors.append("CalculiX section assignment overlaps an existing material section: "
                + sectionAssignment.name);
            continue;
        }

        result.caseData.sectionAssignments.push_back(sectionData);
        usedElementIds.insert(usedElementIds.end(), sectionData.elementIds.begin(), sectionData.elementIds.end());
        std::sort(usedElementIds.begin(), usedElementIds.end());
        usedElementIds.erase(std::unique(usedElementIds.begin(), usedElementIds.end()), usedElementIds.end());
        ++exportedSectionIndex;
    }

    if (!result.caseData.sectionAssignments.empty() && usedElementIds.size() != allIds.size()) {
        result.errors.append(QString("CalculiX section assignments cover %1 of %2 tetrahedral elements; every structural element must have exactly one material section.")
            .arg(static_cast<int>(usedElementIds.size()))
            .arg(static_cast<int>(allIds.size())));
    }

    if (!hadEnabledSectionAssignment && result.caseData.sectionAssignments.empty() && !result.caseData.materials.empty()) {
        CalculiXSectionAssignmentData sectionData;
        sectionData.id = "default_section";
        sectionData.name = "Default Solid Section";
        sectionData.materialId = result.caseData.materials.front().id;
        sectionData.materialName = result.caseData.materials.front().id.trimmed().isEmpty()
            ? result.caseData.materials.front().name
            : result.caseData.materials.front().id;
        sectionData.geometryName = meshObject.sourceGeometryName;
        sectionData.meshName = meshObject.name;
        sectionData.elementSetName = "EALL";
        sectionData.elementIds = allIds;
        result.caseData.sectionAssignments.push_back(sectionData);
        if (result.caseData.materials.size() > 1) {
            result.warnings.append("CalculiX has no enabled section assignment; exported a default full-mesh section with the first structural material.");
        }
    }
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

    const StructuralCase &structuralCase = simulationCase.structuralCase;
    const QString meshName = structuralCase.meshName.trimmed().isEmpty()
        ? simulationCase.meshName
        : structuralCase.meshName;

    if (meshName.trimmed().isEmpty()) {
        result.errors.append("CalculiX case data build failed: simulation case has no mesh.");
        return result;
    }

    const MeshObject *meshObject = projectModel.meshRepository().findMeshByName(meshName);
    if (!meshObject) {
        result.errors.append("CalculiX case data build failed: mesh not found: " + meshName);
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
    meshData.name = meshObject->name;
    meshData.sourceGeometryName = meshObject->sourceGeometryName;
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

    for (const Material &material : structuralCase.materials) {
        CalculiXMaterialData materialData = toCalculiXMaterial(material);
        validateMaterial(materialData, result);
        result.caseData.materials.push_back(materialData);
    }
    if (result.caseData.materials.empty()) {
        result.errors.append("CalculiX case data build failed: no structural material is defined.");
    }

    buildSectionAssignments(structuralCase, *meshObject, result);

    for (const BoundaryCondition &boundaryCondition : structuralCase.constraints) {
        if (boundaryCondition.enabled) {
            result.caseData.boundaries.push_back(toCalculiXBoundary(boundaryCondition));
        }
    }
    if (result.caseData.boundaries.empty()) {
        result.warnings.append("CalculiX structural case has no enabled constraints or load targets.");
    }

    for (const Load &load : structuralCase.loads) {
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
    result.logMessages.append(QString("CalculiX case data: %1 nodes, %2 tetrahedra, %3 material(s), %4 section(s), %5 boundary(ies), %6 load(s).")
        .arg(result.caseData.meshData.nodeCount())
        .arg(result.caseData.meshData.tetraCount())
        .arg(static_cast<int>(result.caseData.materials.size()))
        .arg(static_cast<int>(result.caseData.sectionAssignments.size()))
        .arg(static_cast<int>(result.caseData.boundaries.size()))
        .arg(static_cast<int>(result.caseData.loads.size())));
    return result;
}
