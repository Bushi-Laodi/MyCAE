#include "SolverDataController.h"

#include "BoundaryConditionDialog.h"
#include "LoadDialog.h"
#include "MaterialDialog.h"
#include "SectionAssignmentDialog.h"
#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"
#include "project/ProjectModel.h"
#include "PropertyPanel.h"
#include "solver/SolverDataService.h"
#include "solver/SimulationCase.h"

#include <QMessageBox>

#include <optional>
#include <utility>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

SolverDataControllerResult noProjectResult(const QString &action)
{
    SolverDataControllerResult result;
    result.logMessages.append(action + zh(u8"失败：请先创建或打开工程。"));
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

BoundaryConditionDialogOptions structuralBoundaryConditionDialogOptions(const ProjectModel &projectModel)
{
    BoundaryConditionDialogOptions options = boundaryConditionDialogOptions(projectModel);
    options.allowedTypes = {
        BoundaryConditionType::FixedSupport,
        BoundaryConditionType::Displacement,
        BoundaryConditionType::LoadTarget
    };
    return options;
}

BoundaryConditionDialogOptions cfdBoundaryConditionDialogOptions(const ProjectModel &projectModel)
{
    BoundaryConditionDialogOptions options = boundaryConditionDialogOptions(projectModel);
    options.allowedTypes = {
        BoundaryConditionType::Wall,
        BoundaryConditionType::VelocityInlet,
        BoundaryConditionType::PressureInlet,
        BoundaryConditionType::PressureOutlet,
        BoundaryConditionType::Symmetry
    };
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

LoadDialogOptions structuralLoadDialogOptions(const ProjectModel &projectModel)
{
    LoadDialogOptions options;
    const SolverRepository &solverRepository = projectModel.solverRepository();
    for (const BoundaryCondition &boundaryCondition : solverRepository.boundaryConditions()) {
        if (boundaryCondition.type != BoundaryConditionType::LoadTarget
                && boundaryCondition.type != BoundaryConditionType::Wall) {
            continue;
        }
        LoadBoundaryConditionOption option;
        option.id = boundaryCondition.id;
        option.displayName = boundaryCondition.name.trimmed().isEmpty()
            || boundaryCondition.name == boundaryCondition.id
            ? boundaryCondition.id
            : QString("%1 (%2)").arg(boundaryCondition.name, boundaryCondition.id);
        options.boundaryConditions.push_back(option);
    }
    if (projectModel.selection().kind == SelectionKind::BoundaryCondition) {
        const BoundaryCondition *boundaryCondition =
            solverRepository.findBoundaryConditionById(projectModel.selection().id);
        if (boundaryCondition
                && (boundaryCondition->type == BoundaryConditionType::LoadTarget
                    || boundaryCondition->type == BoundaryConditionType::Wall)) {
            options.defaultBoundaryConditionId = projectModel.selection().id;
        }
    } else if (options.boundaryConditions.size() == 1) {
        options.defaultBoundaryConditionId = options.boundaryConditions.front().id;
    }
    options.allowedTypes = {LoadType::Pressure, LoadType::Force, LoadType::SurfaceForce, LoadType::Gravity};
    return options;
}

LoadDialogOptions cfdFieldValueDialogOptions(const ProjectModel &projectModel)
{
    LoadDialogOptions options = loadDialogOptions(projectModel);
    options.allowedTypes = {LoadType::Velocity, LoadType::Pressure};
    return options;
}

SectionAssignmentDialogOptions sectionAssignmentDialogOptions(const ProjectModel &projectModel)
{
    SectionAssignmentDialogOptions options;
    const SolverRepository &solverRepository = projectModel.solverRepository();

    for (const Material &material : solverRepository.materials()) {
        if (isStructuralMaterial(material)) {
            options.solidMaterials.append(material);
        }
    }
    for (const GeometryObject &geometry : projectModel.geometryRepository().geometryObjects()) {
        options.geometries.append(geometry);
    }
    for (const MeshObject &mesh : projectModel.meshRepository().meshObjects()) {
        options.meshes.append(mesh);
    }

    switch (projectModel.selection().kind) {
    case SelectionKind::Geometry:
        options.defaultGeometryName = projectModel.selection().id;
        break;
    case SelectionKind::Mesh:
        options.defaultMeshName = projectModel.selection().id;
        if (const MeshObject *meshObject = projectModel.findMeshByName(projectModel.selection().id)) {
            options.defaultGeometryName = meshObject->sourceGeometryName;
        }
        break;
    default:
        break;
    }

    if (options.defaultGeometryName.isEmpty() && options.geometries.size() == 1) {
        options.defaultGeometryName = options.geometries.front().name;
    }
    if (options.defaultMeshName.isEmpty() && !options.defaultGeometryName.isEmpty()) {
        for (const MeshObject &mesh : options.meshes) {
            if (mesh.sourceGeometryName == options.defaultGeometryName) {
                options.defaultMeshName = mesh.name;
                break;
            }
        }
    }
    if (options.defaultMeshName.isEmpty() && options.meshes.size() == 1) {
        options.defaultMeshName = options.meshes.front().name;
        if (options.defaultGeometryName.isEmpty()) {
            options.defaultGeometryName = options.meshes.front().sourceGeometryName;
        }
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
    return {zh(u8"材料：已定义 %1 个。").arg(solverRepository.materials().size())};
}

QStringList SolverDataController::showSectionAssignmentCategory(
    ProjectModel &projectModel,
    PropertyPanel *propertyPanel
)
{
    projectModel.setSelection(Selection::category(SelectionKind::SectionAssignmentCategory));
    const SolverRepository &solverRepository = projectModel.solverRepository();
    if (propertyPanel) {
        propertyPanel->showSectionAssignmentCategory(solverRepository.sectionAssignments());
    }
    return {zh(u8"材料分区：已定义 %1 个。").arg(solverRepository.sectionAssignments().size())};
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
    return {zh(u8"边界条件：已定义 %1 个。").arg(solverRepository.boundaryConditions().size())};
}

QStringList SolverDataController::showLoadCategory(ProjectModel &projectModel, PropertyPanel *propertyPanel)
{
    projectModel.setSelection(Selection::category(SelectionKind::LoadCategory));
    const SolverRepository &solverRepository = projectModel.solverRepository();
    if (propertyPanel) {
        propertyPanel->showLoadCategory(solverRepository.loads());
    }
    return {zh(u8"载荷：已定义 %1 个。").arg(solverRepository.loads().size())};
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
        return {zh(u8"材料选择失败：未找到：") + materialId};
    }

    projectModel.setSelection(Selection::item(SelectionKind::Material, material->id, material->name));
    if (propertyPanel) {
        propertyPanel->showMaterial(*material);
    }
    return {zh(u8"材料已选择：") + material->name};
}

QStringList SolverDataController::showSectionAssignment(
    ProjectModel &projectModel,
    PropertyPanel *propertyPanel,
    const QString &sectionAssignmentId
)
{
    const SectionAssignment *sectionAssignment =
        projectModel.solverRepository().findSectionAssignmentById(sectionAssignmentId);
    if (!sectionAssignment) {
        projectModel.clearSolverSelection();
        return {zh(u8"材料分区选择失败：未找到：") + sectionAssignmentId};
    }

    projectModel.setSelection(Selection::item(
        SelectionKind::SectionAssignment,
        sectionAssignment->id,
        sectionAssignment->name
    ));
    if (propertyPanel) {
        propertyPanel->showSectionAssignment(*sectionAssignment);
    }
    return {zh(u8"材料分区已选择：") + sectionAssignment->name};
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
        return {zh(u8"边界条件选择失败：未找到：") + boundaryConditionId};
    }

    projectModel.setSelection(Selection::item(
        SelectionKind::BoundaryCondition,
        boundaryCondition->id,
        boundaryCondition->name
    ));
    if (propertyPanel) {
        const SolverRepository &solverRepository = projectModel.solverRepository();
        propertyPanel->showBoundaryCondition(
            *boundaryCondition,
            solverRepository.faceGroups(),
            solverRepository.loads()
        );
    }
    return {zh(u8"边界条件已选择：") + boundaryCondition->name};
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
        return {zh(u8"载荷选择失败：未找到：") + loadId};
    }

    projectModel.setSelection(Selection::item(SelectionKind::Load, load->id, load->name));
    if (propertyPanel) {
        propertyPanel->showLoad(*load);
    }
    return {zh(u8"载荷已选择：") + load->name};
}

SolverDataControllerResult SolverDataController::createMaterial(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"创建材料"));
    }

    SolverDataControllerResult result;
    const std::optional<Material> newMaterial = MaterialDialog::createMaterial(parent);
    if (!newMaterial) {
        result.logMessages.append(zh(u8"已取消创建材料。"));
        return result;
    }
    return SolverDataService::createMaterial(projectModel, *newMaterial);
}

SolverDataControllerResult SolverDataController::createStructuralMaterial(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"创建结构材料"));
    }

    MaterialDialogOptions options;
    options.fixedDomain = MaterialDomain::Solid;
    const std::optional<Material> newMaterial = MaterialDialog::createMaterial(parent, options);
    if (!newMaterial) {
        SolverDataControllerResult result;
        result.logMessages.append(zh(u8"已取消创建结构材料。"));
        return result;
    }
    return SolverDataService::createMaterial(projectModel, *newMaterial);
}

SolverDataControllerResult SolverDataController::createFluidMaterial(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"创建流体材料"));
    }

    MaterialDialogOptions options;
    options.fixedDomain = MaterialDomain::Fluid;
    const std::optional<Material> newMaterial = MaterialDialog::createMaterial(parent, options);
    if (!newMaterial) {
        SolverDataControllerResult result;
        result.logMessages.append(zh(u8"已取消创建流体材料。"));
        return result;
    }
    return SolverDataService::createMaterial(projectModel, *newMaterial);
}

SolverDataControllerResult SolverDataController::createSectionAssignment(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"创建材料分区"));
    }

    SolverDataControllerResult result;
    SectionAssignmentDialogOptions options = sectionAssignmentDialogOptions(projectModel);
    if (options.solidMaterials.isEmpty()) {
        result.logMessages.append(zh(u8"创建材料分区失败：请先创建至少一个固体材料。"));
        return result;
    }
    if (options.geometries.isEmpty()) {
        result.logMessages.append(zh(u8"创建材料分区失败：请先创建或导入一个几何体。"));
        return result;
    }
    if (options.meshes.isEmpty()) {
        result.logMessages.append(zh(u8"创建材料分区失败：请先生成或导入一个网格。"));
        return result;
    }

    const std::optional<SectionAssignment> newSectionAssignment =
        SectionAssignmentDialog::createSectionAssignment(parent, std::move(options));
    if (!newSectionAssignment) {
        result.logMessages.append(zh(u8"已取消创建材料分区。"));
        return result;
    }
    return SolverDataService::createSectionAssignment(projectModel, *newSectionAssignment);
}

SolverDataControllerResult SolverDataController::createBoundaryCondition(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"创建边界条件"));
    }

    SolverDataControllerResult result;
    const std::optional<BoundaryCondition> newBoundaryCondition =
        BoundaryConditionDialog::createBoundaryCondition(parent, boundaryConditionDialogOptions(projectModel));
    if (!newBoundaryCondition) {
        result.logMessages.append(zh(u8"已取消创建边界条件。"));
        return result;
    }
    return SolverDataService::createBoundaryCondition(projectModel, *newBoundaryCondition);
}

SolverDataControllerResult SolverDataController::createStructuralBoundaryCondition(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"创建结构约束"));
    }

    const std::optional<BoundaryCondition> newBoundaryCondition =
        BoundaryConditionDialog::createBoundaryCondition(parent, structuralBoundaryConditionDialogOptions(projectModel));
    if (!newBoundaryCondition) {
        SolverDataControllerResult result;
        result.logMessages.append(zh(u8"已取消创建结构约束。"));
        return result;
    }
    return SolverDataService::createBoundaryCondition(projectModel, *newBoundaryCondition);
}

SolverDataControllerResult SolverDataController::createCfdBoundaryCondition(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"创建 CFD 边界"));
    }

    const std::optional<BoundaryCondition> newBoundaryCondition =
        BoundaryConditionDialog::createBoundaryCondition(parent, cfdBoundaryConditionDialogOptions(projectModel));
    if (!newBoundaryCondition) {
        SolverDataControllerResult result;
        result.logMessages.append(zh(u8"已取消创建 CFD 边界。"));
        return result;
    }
    return SolverDataService::createBoundaryCondition(projectModel, *newBoundaryCondition);
}

SolverDataControllerResult SolverDataController::createLoad(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"创建载荷"));
    }

    SolverDataControllerResult result;
    const std::optional<Load> newLoad = LoadDialog::createLoad(parent, loadDialogOptions(projectModel));
    if (!newLoad) {
        result.logMessages.append(zh(u8"已取消创建载荷。"));
        return result;
    }
    return SolverDataService::createLoad(projectModel, *newLoad);
}

SolverDataControllerResult SolverDataController::createStructuralLoad(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"创建结构载荷"));
    }

    const std::optional<Load> newLoad = LoadDialog::createLoad(parent, structuralLoadDialogOptions(projectModel));
    if (!newLoad) {
        SolverDataControllerResult result;
        result.logMessages.append(zh(u8"已取消创建结构载荷。"));
        return result;
    }
    return SolverDataService::createLoad(projectModel, *newLoad);
}

SolverDataControllerResult SolverDataController::createCfdFieldValue(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"创建 CFD 场值"));
    }

    const std::optional<Load> newLoad = LoadDialog::createLoad(parent, cfdFieldValueDialogOptions(projectModel));
    if (!newLoad) {
        SolverDataControllerResult result;
        result.logMessages.append(zh(u8"已取消创建 CFD 场值。"));
        return result;
    }
    return SolverDataService::createLoad(projectModel, *newLoad);
}

SolverDataControllerResult SolverDataController::editSelected(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"编辑求解数据"));
    }

    SolverDataControllerResult result;
    if (Material *material = projectModel.materialForSelection()) {
        const QString originalId = material->id;
        const std::optional<Material> editedMaterial = MaterialDialog::editMaterial(parent, *material);
        if (!editedMaterial) {
            result.logMessages.append(zh(u8"已取消编辑材料。"));
            return result;
        }
        return SolverDataService::updateMaterial(projectModel, originalId, *editedMaterial);
    }

    if (SectionAssignment *sectionAssignment = projectModel.sectionAssignmentForSelection()) {
        const QString originalId = sectionAssignment->id;
        const std::optional<SectionAssignment> editedSectionAssignment =
            SectionAssignmentDialog::editSectionAssignment(
                parent,
                *sectionAssignment,
                sectionAssignmentDialogOptions(projectModel)
            );
        if (!editedSectionAssignment) {
            result.logMessages.append(zh(u8"已取消编辑材料分区。"));
            return result;
        }
        return SolverDataService::updateSectionAssignment(projectModel, originalId, *editedSectionAssignment);
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
            result.logMessages.append(zh(u8"已取消编辑边界条件。"));
            return result;
        }
        return SolverDataService::updateBoundaryCondition(projectModel, originalId, *editedBoundaryCondition);
    }

    if (Load *load = projectModel.loadForSelection()) {
        const QString originalId = load->id;
        const std::optional<Load> editedLoad =
            LoadDialog::editLoad(parent, *load, loadDialogOptions(projectModel));
        if (!editedLoad) {
            result.logMessages.append(zh(u8"已取消编辑载荷。"));
            return result;
        }
        return SolverDataService::updateLoad(projectModel, originalId, *editedLoad);
    }

    result.logMessages.append(zh(u8"编辑求解数据失败：请先选择材料、材料分区、边界条件或载荷。"));
    return result;
}

SolverDataControllerResult SolverDataController::deleteSelected(QWidget *parent, ProjectModel &projectModel)
{
    if (!projectModel.hasProject()) {
        return noProjectResult(zh(u8"删除求解数据"));
    }

    SolverDataControllerResult result;
    if (const Material *material = projectModel.materialForSelection()) {
        const QString id = material->id;
        const QString name = material->name;
        const QMessageBox::StandardButton answer = QMessageBox::question(
            parent,
            zh(u8"删除材料"),
            zh(u8"删除材料“%1”？\n使用此材料的边界条件会保留空材料 ID。").arg(name)
        );
        if (answer != QMessageBox::Yes) {
            result.logMessages.append(zh(u8"已取消删除材料。"));
            return result;
        }

        return SolverDataService::deleteMaterial(projectModel, id);
    }

    if (const SectionAssignment *sectionAssignment = projectModel.sectionAssignmentForSelection()) {
        const QString id = sectionAssignment->id;
        const QString name = sectionAssignment->name;
        const QMessageBox::StandardButton answer = QMessageBox::question(
            parent,
            zh(u8"删除材料分区"),
            zh(u8"删除材料分区“%1”？\n删除后该分区不会再导出为 *SOLID SECTION。").arg(name)
        );
        if (answer != QMessageBox::Yes) {
            result.logMessages.append(zh(u8"已取消删除材料分区。"));
            return result;
        }

        return SolverDataService::deleteSectionAssignment(projectModel, id);
    }

    if (const BoundaryCondition *boundaryCondition = projectModel.boundaryConditionForSelection()) {
        const QString id = boundaryCondition->id;
        const QString name = boundaryCondition->name;
        const QMessageBox::StandardButton answer = QMessageBox::question(
            parent,
            zh(u8"删除边界条件"),
            zh(u8"删除边界条件“%1”？\n关联到它的载荷也会被删除。").arg(name)
        );
        if (answer != QMessageBox::Yes) {
            result.logMessages.append(zh(u8"已取消删除边界条件。"));
            return result;
        }

        return SolverDataService::deleteBoundaryCondition(projectModel, id);
    }

    if (const Load *load = projectModel.loadForSelection()) {
        const QString id = load->id;
        const QString name = load->name;
        const QMessageBox::StandardButton answer = QMessageBox::question(
            parent,
            zh(u8"删除载荷"),
            zh(u8"删除载荷“%1”？").arg(name)
        );
        if (answer != QMessageBox::Yes) {
            result.logMessages.append(zh(u8"已取消删除载荷。"));
            return result;
        }

        return SolverDataService::deleteLoad(projectModel, id);
    }

    result.logMessages.append(zh(u8"删除求解数据失败：请先选择材料、材料分区、边界条件或载荷。"));
    return result;
}
