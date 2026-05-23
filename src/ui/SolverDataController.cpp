#include "SolverDataController.h"

#include "BoundaryConditionDialog.h"
#include "LoadDialog.h"
#include "MaterialDialog.h"
#include "geometry/GeometryObject.h"
#include "project/ProjectModel.h"
#include "PropertyPanel.h"

#include <QMessageBox>

#include <algorithm>
#include <optional>

namespace
{
void clearNonSolverSelection(ProjectModel &projectModel)
{
    projectModel.clearSelectedGeometry();
    projectModel.clearSelectedMesh();
}

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

SolverDataControllerResult noProjectResult(const QString &action)
{
    SolverDataControllerResult result;
    result.logMessages.append(action + " failed: create or open a project first.");
    return result;
}

BoundaryConditionDialogOptions boundaryConditionDialogOptions(const ProjectModel &projectModel)
{
    BoundaryConditionDialogOptions options;
    for (const GeometryObject &geometry : projectModel.geometryObjects()) {
        options.geometryNames.append(geometry.name);
        options.faceGroupsByGeometry.insert(geometry.name, {"Default"});
    }

    for (const Material &material : projectModel.materials()) {
        options.materialIds.append(material.id);
    }
    return options;
}
}

QStringList SolverDataController::showMaterialCategory(ProjectModel &projectModel, PropertyPanel *propertyPanel)
{
    clearNonSolverSelection(projectModel);
    projectModel.clearSelectedSolverData();
    if (propertyPanel) {
        propertyPanel->showMaterialCategory(projectModel.materials());
    }
    return {QString("Materials: %1 defined.").arg(projectModel.materials().size())};
}

QStringList SolverDataController::showBoundaryConditionCategory(
    ProjectModel &projectModel,
    PropertyPanel *propertyPanel
)
{
    clearNonSolverSelection(projectModel);
    projectModel.clearSelectedSolverData();
    if (propertyPanel) {
        propertyPanel->showBoundaryConditionCategory(projectModel.boundaryConditions());
    }
    return {QString("Boundary conditions: %1 defined.").arg(projectModel.boundaryConditions().size())};
}

QStringList SolverDataController::showLoadCategory(ProjectModel &projectModel, PropertyPanel *propertyPanel)
{
    clearNonSolverSelection(projectModel);
    projectModel.clearSelectedSolverData();
    if (propertyPanel) {
        propertyPanel->showLoadCategory(projectModel.loads());
    }
    return {QString("Loads: %1 defined.").arg(projectModel.loads().size())};
}

QStringList SolverDataController::showMaterial(
    ProjectModel &projectModel,
    PropertyPanel *propertyPanel,
    const QString &materialId
)
{
    const Material *material = projectModel.findMaterialById(materialId);
    if (!material) {
        projectModel.clearSelectedSolverData();
        return {"Material selection failed: not found: " + materialId};
    }

    projectModel.setSelectedMaterialId(materialId);
    if (propertyPanel) {
        propertyPanel->showMaterial(*material);
    }
    return {"Material selected: " + material->name};
}

QStringList SolverDataController::showBoundaryCondition(
    ProjectModel &projectModel,
    PropertyPanel *propertyPanel,
    const QString &boundaryConditionId
)
{
    const BoundaryCondition *boundaryCondition = projectModel.findBoundaryConditionById(boundaryConditionId);
    if (!boundaryCondition) {
        projectModel.clearSelectedSolverData();
        return {"Boundary condition selection failed: not found: " + boundaryConditionId};
    }

    projectModel.setSelectedBoundaryConditionId(boundaryConditionId);
    if (propertyPanel) {
        propertyPanel->showBoundaryCondition(*boundaryCondition);
    }
    return {"Boundary condition selected: " + boundaryCondition->name};
}

QStringList SolverDataController::showLoad(
    ProjectModel &projectModel,
    PropertyPanel *propertyPanel,
    const QString &loadId
)
{
    const Load *load = projectModel.findLoadById(loadId);
    if (!load) {
        projectModel.clearSelectedSolverData();
        return {"Load selection failed: not found: " + loadId};
    }

    projectModel.setSelectedLoadId(loadId);
    if (propertyPanel) {
        propertyPanel->showLoad(*load);
    }
    return {"Load selected: " + load->name};
}

SolverDataControllerResult SolverDataController::createMaterial(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult("Create material");
    }

    SolverDataControllerResult result;
    const std::optional<Material> newMaterial = MaterialDialog::createMaterial(parent);
    if (!newMaterial) {
        result.logMessages.append("Create material canceled.");
        return result;
    }
    if (hasMaterialId(projectModel.materials(), newMaterial->id)) {
        result.logMessages.append("Create material failed: duplicated material ID: " + newMaterial->id);
        return result;
    }

    projectModel.materials().push_back(*newMaterial);
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::Material;
    result.selectionId = newMaterial->id;
    result.logMessages.append(QString("Material created: %1 (ID: %2)").arg(newMaterial->name, newMaterial->id));
    return result;
}

SolverDataControllerResult SolverDataController::createBoundaryCondition(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult("Create boundary condition");
    }

    SolverDataControllerResult result;
    const std::optional<BoundaryCondition> newBoundaryCondition =
        BoundaryConditionDialog::createBoundaryCondition(parent, boundaryConditionDialogOptions(projectModel));
    if (!newBoundaryCondition) {
        result.logMessages.append("Create boundary condition canceled.");
        return result;
    }
    if (hasBoundaryConditionId(projectModel.boundaryConditions(), newBoundaryCondition->id)) {
        result.logMessages.append(
            "Create boundary condition failed: duplicated boundary condition ID: " + newBoundaryCondition->id
        );
        return result;
    }

    projectModel.boundaryConditions().push_back(*newBoundaryCondition);
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::BoundaryCondition;
    result.selectionId = newBoundaryCondition->id;
    result.logMessages.append(QString("Boundary condition created: %1 (Type: %2)")
        .arg(newBoundaryCondition->name, toString(newBoundaryCondition->type)));
    return result;
}

SolverDataControllerResult SolverDataController::createLoad(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult("Create load");
    }

    SolverDataControllerResult result;
    const std::optional<Load> newLoad = LoadDialog::createLoad(parent);
    if (!newLoad) {
        result.logMessages.append("Create load canceled.");
        return result;
    }
    if (hasLoadId(projectModel.loads(), newLoad->id)) {
        result.logMessages.append("Create load failed: duplicated load ID: " + newLoad->id);
        return result;
    }

    projectModel.loads().push_back(*newLoad);
    result.changed = true;
    result.selectionKind = SolverDataSelectionKind::Load;
    result.selectionId = newLoad->id;
    result.logMessages.append(QString("Load created: %1 (Value: %2)").arg(newLoad->name).arg(newLoad->value.x));
    return result;
}

SolverDataControllerResult SolverDataController::editSelected(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult("Edit solver data");
    }

    SolverDataControllerResult result;
    if (Material *material = projectModel.findMaterialById(projectModel.selectedMaterialId())) {
        const QString originalId = material->id;
        const std::optional<Material> editedMaterial = MaterialDialog::editMaterial(parent, *material);
        if (!editedMaterial) {
            result.logMessages.append("Edit material canceled.");
            return result;
        }
        if (hasMaterialId(projectModel.materials(), editedMaterial->id, originalId)) {
            result.logMessages.append("Edit material failed: duplicated material ID: " + editedMaterial->id);
            return result;
        }

        *material = *editedMaterial;
        replaceMaterialReferences(projectModel.boundaryConditions(), originalId, editedMaterial->id);
        projectModel.setSelectedMaterialId(editedMaterial->id);
        result.changed = true;
        result.selectionKind = SolverDataSelectionKind::Material;
        result.selectionId = editedMaterial->id;
        result.logMessages.append("Material edited: " + editedMaterial->name);
        return result;
    }

    if (BoundaryCondition *boundaryCondition =
            projectModel.findBoundaryConditionById(projectModel.selectedBoundaryConditionId())) {
        const QString originalId = boundaryCondition->id;
        const std::optional<BoundaryCondition> editedBoundaryCondition =
            BoundaryConditionDialog::editBoundaryCondition(
                parent,
                *boundaryCondition,
                boundaryConditionDialogOptions(projectModel)
            );
        if (!editedBoundaryCondition) {
            result.logMessages.append("Edit boundary condition canceled.");
            return result;
        }
        if (hasBoundaryConditionId(projectModel.boundaryConditions(), editedBoundaryCondition->id, originalId)) {
            result.logMessages.append(
                "Edit boundary condition failed: duplicated boundary condition ID: " + editedBoundaryCondition->id
            );
            return result;
        }

        *boundaryCondition = *editedBoundaryCondition;
        replaceBoundaryConditionReferences(projectModel.loads(), originalId, editedBoundaryCondition->id);
        projectModel.setSelectedBoundaryConditionId(editedBoundaryCondition->id);
        result.changed = true;
        result.selectionKind = SolverDataSelectionKind::BoundaryCondition;
        result.selectionId = editedBoundaryCondition->id;
        result.logMessages.append("Boundary condition edited: " + editedBoundaryCondition->name);
        return result;
    }

    if (Load *load = projectModel.findLoadById(projectModel.selectedLoadId())) {
        const QString originalId = load->id;
        const std::optional<Load> editedLoad = LoadDialog::editLoad(parent, *load);
        if (!editedLoad) {
            result.logMessages.append("Edit load canceled.");
            return result;
        }
        if (hasLoadId(projectModel.loads(), editedLoad->id, originalId)) {
            result.logMessages.append("Edit load failed: duplicated load ID: " + editedLoad->id);
            return result;
        }

        *load = *editedLoad;
        projectModel.setSelectedLoadId(editedLoad->id);
        result.changed = true;
        result.selectionKind = SolverDataSelectionKind::Load;
        result.selectionId = editedLoad->id;
        result.logMessages.append("Load edited: " + editedLoad->name);
        return result;
    }

    result.logMessages.append("Edit solver data failed: select a material, boundary condition, or load first.");
    return result;
}

SolverDataControllerResult SolverDataController::deleteSelected(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult("Delete solver data");
    }

    SolverDataControllerResult result;
    if (const Material *material = projectModel.findMaterialById(projectModel.selectedMaterialId())) {
        const QString id = material->id;
        const QString name = material->name;
        const QMessageBox::StandardButton answer = QMessageBox::question(
            parent,
            "Delete Material",
            "Delete material \"" + name + "\"?\nBoundary conditions using this material will keep an empty material ID."
        );
        if (answer != QMessageBox::Yes) {
            result.logMessages.append("Delete material canceled.");
            return result;
        }

        auto &materials = projectModel.materials();
        materials.erase(std::remove_if(materials.begin(), materials.end(), [&id](const Material &candidate) {
            return candidate.id == id;
        }), materials.end());
        replaceMaterialReferences(projectModel.boundaryConditions(), id, {});
        projectModel.clearSelectedSolverData();
        result.changed = true;
        result.selectionKind = SolverDataSelectionKind::MaterialCategory;
        result.logMessages.append("Material deleted: " + name);
        return result;
    }

    if (const BoundaryCondition *boundaryCondition =
            projectModel.findBoundaryConditionById(projectModel.selectedBoundaryConditionId())) {
        const QString id = boundaryCondition->id;
        const QString name = boundaryCondition->name;
        const QMessageBox::StandardButton answer = QMessageBox::question(
            parent,
            "Delete Boundary Condition",
            "Delete boundary condition \"" + name + "\"?\nLoads linked to it will also be deleted."
        );
        if (answer != QMessageBox::Yes) {
            result.logMessages.append("Delete boundary condition canceled.");
            return result;
        }

        auto &boundaryConditions = projectModel.boundaryConditions();
        boundaryConditions.erase(
            std::remove_if(
                boundaryConditions.begin(),
                boundaryConditions.end(),
                [&id](const BoundaryCondition &candidate) {
                    return candidate.id == id;
                }
            ),
            boundaryConditions.end()
        );

        auto &loads = projectModel.loads();
        loads.erase(std::remove_if(loads.begin(), loads.end(), [&id](const Load &candidate) {
            return candidate.boundaryConditionId == id;
        }), loads.end());
        projectModel.clearSelectedSolverData();
        result.changed = true;
        result.selectionKind = SolverDataSelectionKind::BoundaryConditionCategory;
        result.logMessages.append("Boundary condition deleted: " + name);
        return result;
    }

    if (const Load *load = projectModel.findLoadById(projectModel.selectedLoadId())) {
        const QString id = load->id;
        const QString name = load->name;
        const QMessageBox::StandardButton answer = QMessageBox::question(
            parent,
            "Delete Load",
            "Delete load \"" + name + "\"?"
        );
        if (answer != QMessageBox::Yes) {
            result.logMessages.append("Delete load canceled.");
            return result;
        }

        auto &loads = projectModel.loads();
        loads.erase(std::remove_if(loads.begin(), loads.end(), [&id](const Load &candidate) {
            return candidate.id == id;
        }), loads.end());
        projectModel.clearSelectedSolverData();
        result.changed = true;
        result.selectionKind = SolverDataSelectionKind::LoadCategory;
        result.logMessages.append("Load deleted: " + name);
        return result;
    }

    result.logMessages.append("Delete solver data failed: select a material, boundary condition, or load first.");
    return result;
}
