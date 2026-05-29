#include "solver/SolverDataService.h"

#include "project/ProjectModel.h"
#include "solver/SectionAssignment.h"

#include <algorithm>
#include <vector>

namespace
{
bool hasMaterialId(const std::vector<Material> &materials, const QString &id, const QString &excludedId = {})
{
    for (const Material &material : materials) {
        if (material.id == id && material.id != excludedId) {
            return true;
        }
    }
    return false;
}

bool hasBoundaryConditionId(
    const std::vector<BoundaryCondition> &boundaryConditions,
    const QString &id,
    const QString &excludedId = {}
)
{
    for (const BoundaryCondition &boundaryCondition : boundaryConditions) {
        if (boundaryCondition.id == id && boundaryCondition.id != excludedId) {
            return true;
        }
    }
    return false;
}

bool hasLoadId(const std::vector<Load> &loads, const QString &id, const QString &excludedId = {})
{
    for (const Load &load : loads) {
        if (load.id == id && load.id != excludedId) {
            return true;
        }
    }
    return false;
}

bool hasSectionAssignmentId(
    const std::vector<SectionAssignment> &sectionAssignments,
    const QString &id,
    const QString &excludedId = {}
)
{
    for (const SectionAssignment &sectionAssignment : sectionAssignments) {
        if (sectionAssignment.id == id && sectionAssignment.id != excludedId) {
            return true;
        }
    }
    return false;
}

QString sanitizedIdPart(QString text)
{
    text = text.trimmed().toLower();
    for (qsizetype i = 0; i < text.size(); ++i) {
        if (!text.at(i).isLetterOrNumber()) {
            text[i] = '_';
        }
    }
    while (text.contains("__")) {
        text.replace("__", "_");
    }
    while (text.startsWith('_')) {
        text.remove(0, 1);
    }
    while (text.endsWith('_')) {
        text.chop(1);
    }
    return text;
}

QString defaultSectionAssignmentId(const SectionAssignment &sectionAssignment)
{
    const QString materialPart = sanitizedIdPart(sectionAssignment.materialId);
    const QString meshPart = sanitizedIdPart(sectionAssignment.meshName);
    const QString fallbackPart = sanitizedIdPart(sectionAssignment.name);
    QStringList parts{"section"};
    if (!materialPart.isEmpty()) {
        parts.append(materialPart);
    }
    if (!meshPart.isEmpty()) {
        parts.append(meshPart);
    }
    if (parts.size() == 1 && !fallbackPart.isEmpty()) {
        parts.append(fallbackPart);
    }
    return parts.join('_');
}

SectionAssignment normalizedSectionAssignment(SectionAssignment sectionAssignment)
{
    sectionAssignment.name = sectionAssignment.name.trimmed();
    sectionAssignment.materialId = sectionAssignment.materialId.trimmed();
    sectionAssignment.geometryName = sectionAssignment.geometryName.trimmed();
    sectionAssignment.meshName = sectionAssignment.meshName.trimmed();
    sectionAssignment.elementSetName = sectionAssignment.elementSetName.trimmed();
    if (sectionAssignment.elementSetName.isEmpty()) {
        sectionAssignment.elementSetName = "EALL";
    }
    if (sectionAssignment.name.isEmpty()) {
        sectionAssignment.name = QString("%1 @ %2")
            .arg(sectionAssignment.materialId, sectionAssignment.meshName);
    }
    if (sectionAssignment.id.trimmed().isEmpty()) {
        sectionAssignment.id = defaultSectionAssignmentId(sectionAssignment);
    } else {
        sectionAssignment.id = sanitizedIdPart(sectionAssignment.id);
    }
    if (sectionAssignment.id.isEmpty()) {
        sectionAssignment.id = "section_assignment";
    }
    return sectionAssignment;
}

QString uniqueSectionAssignmentId(
    const std::vector<SectionAssignment> &sectionAssignments,
    const QString &baseId
)
{
    if (!hasSectionAssignmentId(sectionAssignments, baseId)) {
        return baseId;
    }
    for (int suffix = 2; suffix < 10000; ++suffix) {
        const QString candidate = QString("%1_%2").arg(baseId).arg(suffix);
        if (!hasSectionAssignmentId(sectionAssignments, candidate)) {
            return candidate;
        }
    }
    return baseId;
}

void replaceMaterialReferences(
    std::vector<BoundaryCondition> &boundaryConditions,
    const QString &oldMaterialId,
    const QString &newMaterialId
)
{
    for (BoundaryCondition &boundaryCondition : boundaryConditions) {
        if (boundaryCondition.materialId == oldMaterialId) {
            boundaryCondition.materialId = newMaterialId;
        }
    }
}

void replaceMaterialReferences(
    std::vector<SectionAssignment> &sectionAssignments,
    const QString &oldMaterialId,
    const QString &newMaterialId
)
{
    for (SectionAssignment &sectionAssignment : sectionAssignments) {
        if (sectionAssignment.materialId == oldMaterialId) {
            sectionAssignment.materialId = newMaterialId;
        }
    }
}

void replaceBoundaryConditionReferences(
    std::vector<Load> &loads,
    const QString &oldBoundaryConditionId,
    const QString &newBoundaryConditionId
)
{
    for (Load &load : loads) {
        if (load.boundaryConditionId == oldBoundaryConditionId) {
            load.boundaryConditionId = newBoundaryConditionId;
        }
    }
}
}

SolverDataServiceResult SolverDataService::createMaterial(ProjectModel &projectModel, const Material &material)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    if (hasMaterialId(solverRepository.materials(), material.id)) {
        result.logMessages.append("Create material failed: duplicated material ID: " + material.id);
        return result;
    }

    solverRepository.materials().push_back(material);
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::Material;
    result.selectionId = material.id;
    result.logMessages.append(QString("Material created: %1 (ID: %2)").arg(material.name, material.id));
    return result;
}

SolverDataServiceResult SolverDataService::createSectionAssignment(
    ProjectModel &projectModel,
    const SectionAssignment &sectionAssignment
)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    SectionAssignment normalized = normalizedSectionAssignment(sectionAssignment);
    normalized.id = uniqueSectionAssignmentId(solverRepository.sectionAssignments(), normalized.id);
    if (!solverRepository.findMaterialById(normalized.materialId)) {
        result.logMessages.append("Create section assignment failed: material does not exist: " + normalized.materialId);
        return result;
    }
    if (!projectModel.findGeometryByName(normalized.geometryName)) {
        result.logMessages.append("Create section assignment failed: geometry does not exist: " + normalized.geometryName);
        return result;
    }
    if (normalized.meshName.isEmpty()) {
        result.logMessages.append("Create section assignment failed: mesh name is empty.");
        return result;
    }
    if (!projectModel.findMeshByName(normalized.meshName)) {
        result.logMessages.append("Create section assignment failed: mesh does not exist: " + normalized.meshName);
        return result;
    }

    solverRepository.sectionAssignments().push_back(normalized);
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::SectionAssignment;
    result.selectionId = normalized.id;
    result.logMessages.append(QString("Section assignment created: %1 (Element set: %2)")
        .arg(normalized.name, normalized.elementSetName));
    return result;
}

SolverDataServiceResult SolverDataService::createBoundaryCondition(
    ProjectModel &projectModel,
    const BoundaryCondition &boundaryCondition
)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    if (hasBoundaryConditionId(solverRepository.boundaryConditions(), boundaryCondition.id)) {
        result.logMessages.append(
            "Create boundary condition failed: duplicated boundary condition ID: " + boundaryCondition.id
        );
        return result;
    }

    solverRepository.boundaryConditions().push_back(boundaryCondition);
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::BoundaryCondition;
    result.selectionId = boundaryCondition.id;
    result.logMessages.append(QString("Boundary condition created: %1 (Type: %2)")
        .arg(boundaryCondition.name, toString(boundaryCondition.type)));
    return result;
}

SolverDataServiceResult SolverDataService::createLoad(ProjectModel &projectModel, const Load &load)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    if (hasLoadId(solverRepository.loads(), load.id)) {
        result.logMessages.append("Create load failed: duplicated load ID: " + load.id);
        return result;
    }

    solverRepository.loads().push_back(load);
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::Load;
    result.selectionId = load.id;
    result.logMessages.append(QString("Load created: %1 (Value: %2)").arg(load.name).arg(load.value.x));
    return result;
}

SolverDataServiceResult SolverDataService::updateMaterial(
    ProjectModel &projectModel,
    const QString &originalId,
    const Material &material
)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    if (hasMaterialId(solverRepository.materials(), material.id, originalId)) {
        result.logMessages.append("Edit material failed: duplicated material ID: " + material.id);
        return result;
    }

    Material *storedMaterial = solverRepository.findMaterialById(originalId);
    if (!storedMaterial) {
        result.logMessages.append("Edit material failed: selected material no longer exists.");
        return result;
    }

    *storedMaterial = material;
    replaceMaterialReferences(solverRepository.boundaryConditions(), originalId, material.id);
    replaceMaterialReferences(solverRepository.sectionAssignments(), originalId, material.id);
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::Material;
    result.selectionId = material.id;
    result.logMessages.append("Material edited: " + material.name);
    return result;
}

SolverDataServiceResult SolverDataService::updateSectionAssignment(
    ProjectModel &projectModel,
    const QString &originalId,
    const SectionAssignment &sectionAssignment
)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    const SectionAssignment normalized = normalizedSectionAssignment(sectionAssignment);
    if (hasSectionAssignmentId(solverRepository.sectionAssignments(), normalized.id, originalId)) {
        result.logMessages.append("Edit section assignment failed: duplicated section assignment ID: " + normalized.id);
        return result;
    }
    if (!solverRepository.findMaterialById(normalized.materialId)) {
        result.logMessages.append("Edit section assignment failed: material does not exist: " + normalized.materialId);
        return result;
    }
    if (!projectModel.findGeometryByName(normalized.geometryName)) {
        result.logMessages.append("Edit section assignment failed: geometry does not exist: " + normalized.geometryName);
        return result;
    }
    if (!projectModel.findMeshByName(normalized.meshName)) {
        result.logMessages.append("Edit section assignment failed: mesh does not exist: " + normalized.meshName);
        return result;
    }

    SectionAssignment *storedSectionAssignment = solverRepository.findSectionAssignmentById(originalId);
    if (!storedSectionAssignment) {
        result.logMessages.append("Edit section assignment failed: selected section assignment no longer exists.");
        return result;
    }

    *storedSectionAssignment = normalized;
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::SectionAssignment;
    result.selectionId = normalized.id;
    result.logMessages.append("Section assignment edited: " + normalized.name);
    return result;
}

SolverDataServiceResult SolverDataService::updateBoundaryCondition(
    ProjectModel &projectModel,
    const QString &originalId,
    const BoundaryCondition &boundaryCondition
)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    if (hasBoundaryConditionId(solverRepository.boundaryConditions(), boundaryCondition.id, originalId)) {
        result.logMessages.append(
            "Edit boundary condition failed: duplicated boundary condition ID: " + boundaryCondition.id
        );
        return result;
    }

    BoundaryCondition *storedBoundaryCondition = solverRepository.findBoundaryConditionById(originalId);
    if (!storedBoundaryCondition) {
        result.logMessages.append("Edit boundary condition failed: selected boundary condition no longer exists.");
        return result;
    }

    *storedBoundaryCondition = boundaryCondition;
    replaceBoundaryConditionReferences(solverRepository.loads(), originalId, boundaryCondition.id);
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::BoundaryCondition;
    result.selectionId = boundaryCondition.id;
    result.logMessages.append("Boundary condition edited: " + boundaryCondition.name);
    return result;
}

SolverDataServiceResult SolverDataService::updateLoad(
    ProjectModel &projectModel,
    const QString &originalId,
    const Load &load
)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    if (hasLoadId(solverRepository.loads(), load.id, originalId)) {
        result.logMessages.append("Edit load failed: duplicated load ID: " + load.id);
        return result;
    }

    Load *storedLoad = solverRepository.findLoadById(originalId);
    if (!storedLoad) {
        result.logMessages.append("Edit load failed: selected load no longer exists.");
        return result;
    }

    *storedLoad = load;
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::Load;
    result.selectionId = load.id;
    result.logMessages.append("Load edited: " + load.name);
    return result;
}

SolverDataServiceResult SolverDataService::deleteMaterial(ProjectModel &projectModel, const QString &materialId)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    const Material *material = solverRepository.findMaterialById(materialId);
    if (!material) {
        result.logMessages.append("Delete material failed: selected material no longer exists.");
        return result;
    }

    const QString name = material->name;
    auto &materials = solverRepository.materials();
    materials.erase(std::remove_if(materials.begin(), materials.end(), [&materialId](const Material &candidate) {
        return candidate.id == materialId;
    }), materials.end());
    replaceMaterialReferences(solverRepository.boundaryConditions(), materialId, {});
    replaceMaterialReferences(solverRepository.sectionAssignments(), materialId, {});
    projectModel.clearSolverSelection();

    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::MaterialCategory;
    result.logMessages.append("Material deleted: " + name);
    return result;
}

SolverDataServiceResult SolverDataService::deleteSectionAssignment(
    ProjectModel &projectModel,
    const QString &sectionAssignmentId
)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    const SectionAssignment *sectionAssignment = solverRepository.findSectionAssignmentById(sectionAssignmentId);
    if (!sectionAssignment) {
        result.logMessages.append("Delete section assignment failed: selected section assignment no longer exists.");
        return result;
    }

    const QString name = sectionAssignment->name;
    auto &sectionAssignments = solverRepository.sectionAssignments();
    sectionAssignments.erase(
        std::remove_if(
            sectionAssignments.begin(),
            sectionAssignments.end(),
            [&sectionAssignmentId](const SectionAssignment &candidate) {
                return candidate.id == sectionAssignmentId;
            }
        ),
        sectionAssignments.end()
    );
    projectModel.clearSolverSelection();

    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::SectionAssignmentCategory;
    result.logMessages.append("Section assignment deleted: " + name);
    return result;
}

SolverDataServiceResult SolverDataService::deleteBoundaryCondition(
    ProjectModel &projectModel,
    const QString &boundaryConditionId
)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    const BoundaryCondition *boundaryCondition = solverRepository.findBoundaryConditionById(boundaryConditionId);
    if (!boundaryCondition) {
        result.logMessages.append("Delete boundary condition failed: selected boundary condition no longer exists.");
        return result;
    }

    const QString name = boundaryCondition->name;
    auto &boundaryConditions = solverRepository.boundaryConditions();
    boundaryConditions.erase(
        std::remove_if(
            boundaryConditions.begin(),
            boundaryConditions.end(),
            [&boundaryConditionId](const BoundaryCondition &candidate) {
                return candidate.id == boundaryConditionId;
            }
        ),
        boundaryConditions.end()
    );

    auto &loads = solverRepository.loads();
    loads.erase(std::remove_if(loads.begin(), loads.end(), [&boundaryConditionId](const Load &candidate) {
        return candidate.boundaryConditionId == boundaryConditionId;
    }), loads.end());
    projectModel.clearSolverSelection();

    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::BoundaryConditionCategory;
    result.logMessages.append("Boundary condition deleted: " + name);
    return result;
}

SolverDataServiceResult SolverDataService::deleteLoad(ProjectModel &projectModel, const QString &loadId)
{
    SolverDataServiceResult result;
    SolverRepository &solverRepository = projectModel.solverRepository();
    const Load *load = solverRepository.findLoadById(loadId);
    if (!load) {
        result.logMessages.append("Delete load failed: selected load no longer exists.");
        return result;
    }

    const QString name = load->name;
    auto &loads = solverRepository.loads();
    loads.erase(std::remove_if(loads.begin(), loads.end(), [&loadId](const Load &candidate) {
        return candidate.id == loadId;
    }), loads.end());
    projectModel.clearSolverSelection();

    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::LoadCategory;
    result.logMessages.append("Load deleted: " + name);
    return result;
}
