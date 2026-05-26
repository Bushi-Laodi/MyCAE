#include "SolverDataController.h"

#include "BoundaryConditionDialog.h"
#include "LoadDialog.h"
#include "MaterialDialog.h"
#include "geometry/GeometryObject.h"
#include "project/ProjectModel.h"
#include "PropertyPanel.h"
#include "solver/SolverDataService.h"

#include <QMessageBox>

#include <optional>

namespace
{
SolverDataControllerResult noProjectResult(const QString &action)
{
    SolverDataControllerResult result;
    result.logMessages.append(action + " failed: create or open a project first.");
    return result;
}

BoundaryConditionDialogOptions boundaryConditionDialogOptions(const ProjectModel &projectModel)
{
    BoundaryConditionDialogOptions options;
    const GeometryRepository &geometryRepository = projectModel.geometryRepository();
    const SolverRepository &solverRepository = projectModel.solverRepository();
    for (const GeometryObject &geometry : geometryRepository.geometryObjects()) {
        options.geometryNames.append(geometry.name);
    }
    for (const FaceGroup &faceGroup : solverRepository.faceGroups()) {
        options.faceGroupsByGeometry[faceGroup.geometryName].append(faceGroup.id);
    }
    if (projectModel.selection().kind == SelectionKind::FaceGroup) {
        if (const FaceGroup *faceGroup = solverRepository.findFaceGroupById(projectModel.selection().id)) {
            options.defaultGeometryName = faceGroup->geometryName;
            options.defaultFaceGroupId = faceGroup->id;
        }
    }

    for (const Material &material : solverRepository.materials()) {
        options.materialIds.append(material.id);
    }
    return options;
}

LoadDialogOptions loadDialogOptions(const ProjectModel &projectModel)
{
    LoadDialogOptions options;
    const SolverRepository &solverRepository = projectModel.solverRepository();
    for (const BoundaryCondition &boundaryCondition : solverRepository.boundaryConditions()) {
        LoadBoundaryConditionOption option;
        option.id = boundaryCondition.id;
        option.displayName = boundaryCondition.name.trimmed().isEmpty()
            || boundaryCondition.name == boundaryCondition.id
            ? boundaryCondition.id
            : QString("%1 (%2)").arg(boundaryCondition.name, boundaryCondition.id);
        options.boundaryConditions.push_back(option);
    }

    if (projectModel.selection().kind == SelectionKind::BoundaryCondition) {
        options.defaultBoundaryConditionId = projectModel.selection().id;
    } else if (options.boundaryConditions.size() == 1) {
        options.defaultBoundaryConditionId = options.boundaryConditions.front().id;
    }

    return options;
}
}

QStringList SolverDataController::showMaterialCategory(ProjectModel &projectModel, PropertyPanel *propertyPanel)
{
    projectModel.setSelection(Selection::category(SelectionKind::MaterialCategory));
    const SolverRepository &solverRepository = projectModel.solverRepository();
    if (propertyPanel) {
        propertyPanel->showMaterialCategory(solverRepository.materials());
    }
    return {QString("Materials: %1 defined.").arg(solverRepository.materials().size())};
}

QStringList SolverDataController::showBoundaryConditionCategory(
    ProjectModel &projectModel,
    PropertyPanel *propertyPanel
)
{
    projectModel.setSelection(Selection::category(SelectionKind::BoundaryConditionCategory));
    const SolverRepository &solverRepository = projectModel.solverRepository();
    if (propertyPanel) {
        propertyPanel->showBoundaryConditionCategory(solverRepository.boundaryConditions());
    }
    return {QString("Boundary conditions: %1 defined.").arg(solverRepository.boundaryConditions().size())};
}

QStringList SolverDataController::showLoadCategory(ProjectModel &projectModel, PropertyPanel *propertyPanel)
{
    projectModel.setSelection(Selection::category(SelectionKind::LoadCategory));
    const SolverRepository &solverRepository = projectModel.solverRepository();
    if (propertyPanel) {
        propertyPanel->showLoadCategory(solverRepository.loads());
    }
    return {QString("Loads: %1 defined.").arg(solverRepository.loads().size())};
}

QStringList SolverDataController::showMaterial(
    ProjectModel &projectModel,
    PropertyPanel *propertyPanel,
    const QString &materialId
)
{
    const Material *material = projectModel.solverRepository().findMaterialById(materialId);
    if (!material) {
        projectModel.clearSolverSelection();
        return {"Material selection failed: not found: " + materialId};
    }

    projectModel.setSelection(Selection::item(SelectionKind::Material, material->id, material->name));
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
    const BoundaryCondition *boundaryCondition =
        projectModel.solverRepository().findBoundaryConditionById(boundaryConditionId);
    if (!boundaryCondition) {
        projectModel.clearSolverSelection();
        return {"Boundary condition selection failed: not found: " + boundaryConditionId};
    }

    projectModel.setSelection(Selection::item(
        SelectionKind::BoundaryCondition,
        boundaryCondition->id,
        boundaryCondition->name
    ));
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
    const Load *load = projectModel.solverRepository().findLoadById(loadId);
    if (!load) {
        projectModel.clearSolverSelection();
        return {"Load selection failed: not found: " + loadId};
    }

    projectModel.setSelection(Selection::item(SelectionKind::Load, load->id, load->name));
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
    return SolverDataService::createMaterial(projectModel, *newMaterial);
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
    return SolverDataService::createBoundaryCondition(projectModel, *newBoundaryCondition);
}

SolverDataControllerResult SolverDataController::createLoad(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult("Create load");
    }

    SolverDataControllerResult result;
    const std::optional<Load> newLoad = LoadDialog::createLoad(parent, loadDialogOptions(projectModel));
    if (!newLoad) {
        result.logMessages.append("Create load canceled.");
        return result;
    }
    return SolverDataService::createLoad(projectModel, *newLoad);
}

SolverDataControllerResult SolverDataController::editSelected(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult("Edit solver data");
    }

    SolverDataControllerResult result;
    if (Material *material = projectModel.materialForSelection()) {
        const QString originalId = material->id;
        const std::optional<Material> editedMaterial = MaterialDialog::editMaterial(parent, *material);
        if (!editedMaterial) {
            result.logMessages.append("Edit material canceled.");
            return result;
        }
        return SolverDataService::updateMaterial(projectModel, originalId, *editedMaterial);
    }

    if (BoundaryCondition *boundaryCondition = projectModel.boundaryConditionForSelection()) {
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
        return SolverDataService::updateBoundaryCondition(projectModel, originalId, *editedBoundaryCondition);
    }

    if (Load *load = projectModel.loadForSelection()) {
        const QString originalId = load->id;
        const std::optional<Load> editedLoad =
            LoadDialog::editLoad(parent, *load, loadDialogOptions(projectModel));
        if (!editedLoad) {
            result.logMessages.append("Edit load canceled.");
            return result;
        }
        return SolverDataService::updateLoad(projectModel, originalId, *editedLoad);
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
    if (const Material *material = projectModel.materialForSelection()) {
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

        return SolverDataService::deleteMaterial(projectModel, id);
    }

    if (const BoundaryCondition *boundaryCondition = projectModel.boundaryConditionForSelection()) {
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

        return SolverDataService::deleteBoundaryCondition(projectModel, id);
    }

    if (const Load *load = projectModel.loadForSelection()) {
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

        return SolverDataService::deleteLoad(projectModel, id);
    }

    result.logMessages.append("Delete solver data failed: select a material, boundary condition, or load first.");
    return result;
}
