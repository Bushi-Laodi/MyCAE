#include "solver/calculix/CalculiXSectionAssignmentValidator.h"

#include "mesh/MeshData.h"
#include "mesh/MeshObject.h"
#include "solver/Material.h"
#include "solver/SimulationCase.h"

#include <algorithm>

namespace
{
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

const Material *findMaterialById(const std::vector<Material> &materials, const QString &materialId)
{
    for (const Material &material : materials) {
        if (material.id == materialId) {
            return &material;
        }
    }
    return nullptr;
}

QString calculixMaterialName(const Material &material)
{
    return material.id.trimmed().isEmpty() ? material.name : material.id;
}

bool physicalGroupMatches(const MeshPhysicalGroup &physicalGroup, const QString &elementSetName)
{
    if (physicalGroup.dimension != 3) {
        return false;
    }

    const QString target = elementSetName.trimmed();
    bool targetIsNumber = false;
    const int targetTag = target.toInt(&targetIsNumber);
    bool physicalVolumeTagOk = false;
    const int physicalVolumeTag = target.startsWith("PhysicalVolume_", Qt::CaseInsensitive)
        ? target.mid(QString("PhysicalVolume_").size()).toInt(&physicalVolumeTagOk)
        : -1;
    bool tagAliasOk = false;
    const int tagAlias = target.startsWith("tag_", Qt::CaseInsensitive)
        ? target.mid(QString("tag_").size()).toInt(&tagAliasOk)
        : -1;
    return (targetIsNumber && physicalGroup.tag == targetTag)
        || (physicalVolumeTagOk && physicalGroup.tag == physicalVolumeTag)
        || (tagAliasOk && physicalGroup.tag == tagAlias)
        || physicalGroup.name.compare(target, Qt::CaseInsensitive) == 0;
}

const MeshPhysicalGroup *findVolumePhysicalGroup(const MeshData &meshData, const QString &elementSetName)
{
    if (elementSetName.trimmed().isEmpty()) {
        return nullptr;
    }

    for (const MeshPhysicalGroup &physicalGroup : meshData.physicalGroups) {
        if (physicalGroupMatches(physicalGroup, elementSetName)) {
            return &physicalGroup;
        }
    }
    return nullptr;
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
            && sectionAssignment.geometryName != meshObject.sourceGeometryName) {
        return false;
    }
    return true;
}

SectionAssignment defaultSectionAssignment(const StructuralCase &structuralCase, const MeshObject &meshObject)
{
    SectionAssignment sectionAssignment;
    sectionAssignment.id = "default_section";
    sectionAssignment.name = "Default Solid Section";
    sectionAssignment.materialId = structuralCase.materials.front().id;
    sectionAssignment.geometryName = meshObject.sourceGeometryName;
    sectionAssignment.meshName = meshObject.name;
    sectionAssignment.elementSetName = "EALL";
    sectionAssignment.enabled = true;
    return sectionAssignment;
}

std::vector<SectionAssignment> enabledSectionAssignments(
    const StructuralCase &structuralCase,
    const MeshObject &meshObject
)
{
    std::vector<SectionAssignment> sectionAssignments;
    for (const SectionAssignment &sectionAssignment : structuralCase.sectionAssignments) {
        if (sectionAssignment.enabled) {
            sectionAssignments.push_back(sectionAssignment);
        }
    }

    if (sectionAssignments.empty() && structuralCase.materials.size() == 1) {
        sectionAssignments.push_back(defaultSectionAssignment(structuralCase, meshObject));
    }
    return sectionAssignments;
}

void appendCoverageError(
    CalculiXSectionValidationResult &result,
    int coveredCount,
    int totalCount
)
{
    result.errors.append(QString(
        "CalculiX section assignments cover %1 of %2 tetrahedral elements; every structural element must have exactly one material section."
    ).arg(coveredCount).arg(totalCount));
}
}

CalculiXSectionValidationResult CalculiXSectionAssignmentValidator::validate(
    const StructuralCase &structuralCase,
    const MeshObject &meshObject,
    const MeshData &meshData
)
{
    CalculiXSectionValidationResult result;
    const std::vector<int> allIds = allElementIds(meshData);
    if (allIds.empty()) {
        result.errors.append("CalculiX section assignment validation failed: mesh has no tetrahedral elements.");
        return result;
    }
    if (structuralCase.materials.empty()) {
        result.errors.append("CalculiX section assignment validation failed: no structural material is defined.");
        return result;
    }

    const bool hasEnabledSectionAssignment = std::any_of(
        structuralCase.sectionAssignments.begin(),
        structuralCase.sectionAssignments.end(),
        [](const SectionAssignment &sectionAssignment) {
            return sectionAssignment.enabled;
        }
    );
    if (!hasEnabledSectionAssignment && structuralCase.materials.size() > 1) {
        result.errors.append("CalculiX section assignment validation failed: multiple structural materials exist but no explicit section assignment is defined.");
        return result;
    }

    std::vector<int> usedElementIds;
    const std::vector<SectionAssignment> sectionAssignments =
        enabledSectionAssignments(structuralCase, meshObject);
    for (const SectionAssignment &sectionAssignment : sectionAssignments) {
        if (!sectionTargetsCurrentMesh(sectionAssignment, meshObject, meshData)) {
            result.errors.append("CalculiX section assignment targets another mesh or geometry: "
                + sectionAssignment.name);
            continue;
        }

        const Material *material = findMaterialById(structuralCase.materials, sectionAssignment.materialId);
        if (!material) {
            result.errors.append("CalculiX section assignment references missing material: "
                + sectionAssignment.name
                + " / materialId=" + sectionAssignment.materialId);
            continue;
        }

        CalculiXSectionValidationItem item;
        item.assignment = sectionAssignment;
        item.elementSetName = isAllElementSetName(sectionAssignment.elementSetName)
            ? QString("EALL")
            : sectionAssignment.elementSetName.trimmed();
        item.materialId = material->id;
        item.materialName = calculixMaterialName(*material);

        if (isAllElementSetName(sectionAssignment.elementSetName)) {
            item.elementIds = allIds;
        } else {
            const MeshPhysicalGroup *physicalGroup =
                findVolumePhysicalGroup(meshData, sectionAssignment.elementSetName);
            if (!physicalGroup) {
                result.errors.append("CalculiX section assignment elementSetName does not match a 3D physical group: "
                    + sectionAssignment.name
                    + " / elementSetName=" + sectionAssignment.elementSetName);
                continue;
            }
            item.elementIds = elementIdsForPhysicalGroup(meshData, physicalGroup->tag);
        }

        if (item.elementIds.empty()) {
            result.errors.append("CalculiX section assignment has no matching volume elements: "
                + sectionAssignment.name
                + " / elementSetName=" + item.elementSetName);
            continue;
        }

        bool overlapsExistingSection = false;
        for (const int elementId : item.elementIds) {
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

        usedElementIds.insert(usedElementIds.end(), item.elementIds.begin(), item.elementIds.end());
        std::sort(usedElementIds.begin(), usedElementIds.end());
        usedElementIds.erase(std::unique(usedElementIds.begin(), usedElementIds.end()), usedElementIds.end());
        result.items.push_back(item);
    }

    if (!result.items.empty() && usedElementIds.size() != allIds.size()) {
        appendCoverageError(
            result,
            static_cast<int>(usedElementIds.size()),
            static_cast<int>(allIds.size())
        );
    }
    if (result.items.empty() && result.errors.empty()) {
        result.errors.append("CalculiX section assignment validation failed: no material section can be exported.");
    }

    result.success = result.errors.empty();
    return result;
}
