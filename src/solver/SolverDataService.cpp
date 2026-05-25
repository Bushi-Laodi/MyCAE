#include "solver/SolverDataService.h"

#include "project/ProjectModel.h"

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
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::Material;
    result.selectionId = material.id;
    result.logMessages.append("Material edited: " + material.name);
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
    projectModel.clearSolverSelection();

    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::MaterialCategory;
    result.logMessages.append("Material deleted: " + name);
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
