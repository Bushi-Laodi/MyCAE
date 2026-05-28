#include "workflow/GeometryWorkflowController.h"

#include "geometry/GeometryBooleanController.h"
#include "geometry/GeometryManager.h"
#include "mesh/MeshManager.h"
#include "project/ProjectModel.h"
#include "result/ResultManager.h"
#include "workflow/ProjectWorkflowController.h"
#include "ui/PropertyPanel.h"
#include "ui/RenderView.h"
#include "workflow/SelectionController.h"

#include <algorithm>

#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString absoluteProjectPath(const ProjectModel &projectModel, const QString &filePath)
{
    if (filePath.trimmed().isEmpty()) {
        return QString();
    }
    const QFileInfo info(filePath);
    return info.isAbsolute()
        ? info.absoluteFilePath()
        : QDir(projectModel.project().rootPath).filePath(filePath);
}

bool removeFileIfExists(const QString &filePath, QStringList *deletedFiles)
{
    if (filePath.trimmed().isEmpty() || !QFileInfo::exists(filePath)) {
        return true;
    }
    if (!QFile::remove(filePath)) {
        return false;
    }
    if (deletedFiles) {
        deletedFiles->append(filePath);
    }
    return true;
}
}

GeometryWorkflowController::GeometryWorkflowController(
    GeometryManager &geometryManager,
    ProjectModel &projectModel,
    ProjectWorkflowController &projectWorkflow,
    PropertyPanel *propertyPanel,
    RenderView *renderView,
    QWidget *parent
)
    : m_geometryManager(geometryManager)
    , m_projectModel(projectModel)
    , m_projectWorkflow(projectWorkflow)
    , m_propertyPanel(propertyPanel)
    , m_renderView(renderView)
    , m_parent(parent)
{
}

GeometryWorkflowResult GeometryWorkflowController::createGeometry(GeometryCreateType type) const
{
    GeometryWorkflowResult workflowResult;
    const GeometryCreationController creationController(m_geometryManager);
    const GeometryCreationResult creationResult =
        creationController.createGeometry(m_parent, m_projectModel.project(), type);

    workflowResult.logMessages.append(creationResult.logMessages);
    if (creationResult.canceled) {
        workflowResult.canceled = true;
        return workflowResult;
    }
    if (!creationResult.success) {
        QMessageBox::warning(m_parent, zh(u8"创建几何失败"), creationResult.errorMessage);
        return workflowResult;
    }

    const QString createdGeometryName = creationResult.geometryObject.name;
    workflowResult.logMessages.append(m_projectWorkflow.loadGeometries().logMessages);
    m_projectWorkflow.refreshProjectTree();

    const SelectionController selectionController(m_projectModel, m_propertyPanel, m_renderView);
    const SelectionControllerResult selectionResult =
        selectionController.apply(Selection::item(SelectionKind::Geometry, createdGeometryName, createdGeometryName));
    workflowResult.logMessages.append(selectionResult.logMessages);
    if (!selectionResult.accepted) {
        workflowResult.logMessages.append(zh(u8"几何已保存，但无法选中：") + createdGeometryName);
    }

    workflowResult.logMessages.append(m_projectWorkflow.saveSimulationCase().logMessages);
    workflowResult.success = true;
    return workflowResult;
}

GeometryWorkflowResult GeometryWorkflowController::createBooleanGeometry() const
{
    GeometryWorkflowResult workflowResult;
    const GeometryBooleanController booleanController(m_geometryManager);
    const GeometryBooleanResult booleanResult = booleanController.createBooleanGeometry(m_parent, m_projectModel);

    workflowResult.logMessages.append(booleanResult.logMessages);
    if (booleanResult.canceled) {
        workflowResult.canceled = true;
        return workflowResult;
    }
    if (!booleanResult.success) {
        QMessageBox::warning(m_parent, zh(u8"布尔操作失败"), booleanResult.errorMessage);
        return workflowResult;
    }

    const QString createdGeometryName = booleanResult.geometryObject.name;
    workflowResult.logMessages.append(m_projectWorkflow.loadGeometries().logMessages);
    m_projectWorkflow.refreshProjectTree();

    const SelectionController selectionController(m_projectModel, m_propertyPanel, m_renderView);
    const SelectionControllerResult selectionResult =
        selectionController.apply(Selection::item(SelectionKind::Geometry, createdGeometryName, createdGeometryName));
    workflowResult.logMessages.append(selectionResult.logMessages);
    if (!selectionResult.accepted) {
        workflowResult.logMessages.append(zh(u8"布尔几何已保存，但无法选中：") + createdGeometryName);
    }

    workflowResult.logMessages.append(m_projectWorkflow.saveSimulationCase().logMessages);
    workflowResult.success = true;
    return workflowResult;
}

GeometryWorkflowResult GeometryWorkflowController::importStepGeometry() const
{
    GeometryWorkflowResult workflowResult;
    if (m_projectModel.project().rootPath.isEmpty()) {
        QMessageBox::warning(m_parent, zh(u8"导入 STEP 失败"), zh(u8"请先新建或打开工程。"));
        workflowResult.logMessages.append("Import STEP failed: no project is open.");
        return workflowResult;
    }

    const QString filePath = QFileDialog::getOpenFileName(
        m_parent,
        zh(u8"导入 STEP"),
        QString(),
        zh(u8"STEP 文件 (*.step *.stp);;所有文件 (*.*)")
    );
    if (filePath.isEmpty()) {
        workflowResult.canceled = true;
        workflowResult.logMessages.append("Import STEP canceled.");
        return workflowResult;
    }

    GeometryObject importedGeometry;
    QString errorMessage;
    if (!m_geometryManager.importStepGeometry(
            m_projectModel.project(),
            filePath,
            &importedGeometry,
            &errorMessage
        )) {
        QMessageBox::warning(m_parent, zh(u8"导入 STEP 失败"), errorMessage);
        workflowResult.logMessages.append("Import STEP failed: " + errorMessage);
        return workflowResult;
    }

    const QString importedGeometryName = importedGeometry.name;
    workflowResult.logMessages.append("STEP imported: " + filePath);
    workflowResult.logMessages.append("Imported geometry created: " + importedGeometryName);
    workflowResult.logMessages.append(m_projectWorkflow.loadGeometries().logMessages);
    m_projectWorkflow.refreshProjectTree();

    const SelectionController selectionController(m_projectModel, m_propertyPanel, m_renderView);
    const SelectionControllerResult selectionResult =
        selectionController.apply(Selection::item(SelectionKind::Geometry, importedGeometryName, importedGeometryName));
    workflowResult.logMessages.append(selectionResult.logMessages);
    if (!selectionResult.accepted) {
        workflowResult.logMessages.append(zh(u8"STEP 已导入，但无法选中：") + importedGeometryName);
    }

    workflowResult.logMessages.append(m_projectWorkflow.saveSimulationCase().logMessages);
    workflowResult.success = true;
    return workflowResult;
}

GeometryWorkflowResult GeometryWorkflowController::deleteSelectedGeometry() const
{
    GeometryWorkflowResult workflowResult;
    const GeometryObject *selectedGeometry = m_projectModel.geometryForSelection();
    if (!selectedGeometry) {
        workflowResult.logMessages.append(zh(u8"未删除几何：当前未选择几何体。"));
        return workflowResult;
    }

    const GeometryObject geometry = *selectedGeometry;
    const QString geometryName = geometry.name;

    int dependentMeshCount = 0;
    for (const MeshObject &meshObject : m_projectModel.meshObjects()) {
        if (meshObject.sourceGeometryName == geometryName) {
            ++dependentMeshCount;
        }
    }

    int dependentFaceGroupCount = 0;
    for (const FaceGroup &faceGroup : m_projectModel.faceGroups()) {
        if (faceGroup.geometryName == geometryName) {
            ++dependentFaceGroupCount;
        }
    }

    int dependentBoundaryConditionCount = 0;
    for (const BoundaryCondition &boundaryCondition : m_projectModel.boundaryConditions()) {
        if (boundaryCondition.target.geometryName == geometryName) {
            ++dependentBoundaryConditionCount;
        }
    }

    const QString message = zh(u8"确定删除几何体“%1”吗？\n\n将同步移除关联的网格、面组、边界条件、载荷和结果历史索引。")
        .arg(geometryName)
        + zh(u8"\n\n关联网格：%1\n关联面组：%2\n关联边界条件：%3")
            .arg(dependentMeshCount)
            .arg(dependentFaceGroupCount)
            .arg(dependentBoundaryConditionCount);
    if (QMessageBox::question(m_parent, zh(u8"删除几何体"), message) != QMessageBox::Yes) {
        workflowResult.canceled = true;
        workflowResult.logMessages.append(zh(u8"已取消删除几何体：") + geometryName);
        return workflowResult;
    }

    QStringList deletedFiles;
    QString errorMessage;
    if (!m_geometryManager.deleteGeometry(m_projectModel.project(), geometry, &deletedFiles, &errorMessage)) {
        QMessageBox::warning(m_parent, zh(u8"删除几何体失败"), errorMessage);
        workflowResult.logMessages.append("Delete geometry failed: " + errorMessage);
        return workflowResult;
    }

    MeshManager meshManager(m_projectModel.project().rootPath);
    QStringList removedMeshNames;
    for (const MeshObject &meshObject : m_projectModel.meshObjects()) {
        if (meshObject.sourceGeometryName != geometryName) {
            continue;
        }
        removedMeshNames.append(meshObject.name);
        const QString meshPath = absoluteProjectPath(m_projectModel, meshObject.mshFile);
        if (!removeFileIfExists(meshPath, &deletedFiles)) {
            workflowResult.logMessages.append("Mesh file delete failed: " + meshPath);
        }
        const QString meshJsonPath = meshManager.meshJsonPathForGeometry(meshObject.sourceGeometryName);
        if (!removeFileIfExists(meshJsonPath, &deletedFiles)) {
            workflowResult.logMessages.append("Mesh JSON delete failed: " + meshJsonPath);
        }
    }

    auto &meshObjects = m_projectModel.meshObjects();
    meshObjects.erase(
        std::remove_if(meshObjects.begin(), meshObjects.end(), [&](const MeshObject &meshObject) {
            return meshObject.sourceGeometryName == geometryName;
        }),
        meshObjects.end()
    );

    auto &meshBoundaries = m_projectModel.meshRepository().meshBoundaries();
    meshBoundaries.erase(
        std::remove_if(meshBoundaries.begin(), meshBoundaries.end(), [&](const MeshBoundary &meshBoundary) {
            return meshBoundary.sourceGeometryName == geometryName || removedMeshNames.contains(meshBoundary.meshName);
        }),
        meshBoundaries.end()
    );

    QStringList removedFaceGroupIds;
    auto &faceGroups = m_projectModel.faceGroups();
    for (const FaceGroup &faceGroup : faceGroups) {
        if (faceGroup.geometryName == geometryName) {
            removedFaceGroupIds.append(faceGroup.id);
        }
    }
    faceGroups.erase(
        std::remove_if(faceGroups.begin(), faceGroups.end(), [&](const FaceGroup &faceGroup) {
            return faceGroup.geometryName == geometryName;
        }),
        faceGroups.end()
    );

    QStringList removedBoundaryConditionIds;
    auto &boundaryConditions = m_projectModel.boundaryConditions();
    for (const BoundaryCondition &boundaryCondition : boundaryConditions) {
        if (boundaryCondition.target.geometryName == geometryName
            || removedFaceGroupIds.contains(boundaryCondition.target.faceGroupId)) {
            removedBoundaryConditionIds.append(boundaryCondition.id);
        }
    }
    boundaryConditions.erase(
        std::remove_if(boundaryConditions.begin(), boundaryConditions.end(), [&](const BoundaryCondition &boundaryCondition) {
            return boundaryCondition.target.geometryName == geometryName
                || removedFaceGroupIds.contains(boundaryCondition.target.faceGroupId);
        }),
        boundaryConditions.end()
    );

    auto &loads = m_projectModel.loads();
    loads.erase(
        std::remove_if(loads.begin(), loads.end(), [&](const Load &load) {
            return removedBoundaryConditionIds.contains(load.boundaryConditionId);
        }),
        loads.end()
    );

    auto &results = m_projectModel.resultRepository().results();
    results.erase(
        std::remove_if(results.begin(), results.end(), [&](const ResultObject &result) {
            return removedMeshNames.contains(result.meshName);
        }),
        results.end()
    );
    ResultManager resultManager;
    if (!resultManager.save(m_projectModel.project(), results, &errorMessage)) {
        workflowResult.logMessages.append("Result index update failed: " + errorMessage);
    }

    workflowResult.logMessages.append(zh(u8"几何体已删除：") + geometryName);
    for (const QString &filePath : deletedFiles) {
        workflowResult.logMessages.append(zh(u8"已删除文件：") + filePath);
    }

    workflowResult.logMessages.append(m_projectWorkflow.loadGeometries().logMessages);
    workflowResult.logMessages.append(m_projectWorkflow.loadMeshes().logMessages);
    workflowResult.logMessages.append(m_projectWorkflow.loadResults().logMessages);
    workflowResult.logMessages.append(m_projectWorkflow.saveSimulationCase().logMessages);
    m_projectWorkflow.refreshProjectTree();

    const SelectionController selectionController(m_projectModel, m_propertyPanel, m_renderView);
    workflowResult.logMessages.append(selectionController.apply(Selection::none()).logMessages);

    workflowResult.success = true;
    return workflowResult;
}
