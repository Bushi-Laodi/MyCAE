#include "workflow/ProjectWorkflowController.h"

#include "geometry/GeometryManager.h"
#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"
#include "project/Project.h"
#include "project/ProjectManager.h"
#include "project/ProjectModel.h"
#include "project/ProjectModelLoader.h"
#include "solver/SimulationCaseManager.h"
#include "ui/ProjectTreePanel.h"
#include "ui/PropertyPanel.h"
#include "ui/RenderView.h"

#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QStatusBar>
#include <QStringList>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}
}

ProjectWorkflowController::ProjectWorkflowController(
    ProjectManager &projectManager,
    const GeometryManager &geometryManager,
    ProjectModel &projectModel,
    ProjectTreePanel *projectTreePanel,
    PropertyPanel *propertyPanel,
    RenderView *renderView,
    QMainWindow *window
)
    : m_projectManager(projectManager)
    , m_geometryManager(geometryManager)
    , m_projectModel(projectModel)
    , m_projectTreePanel(projectTreePanel)
    , m_propertyPanel(propertyPanel)
    , m_renderView(renderView)
    , m_window(window)
{
}

ProjectWorkflowResult ProjectWorkflowController::createProject() const
{
    ProjectWorkflowResult result;
    const QString projectPath = QFileDialog::getExistingDirectory(
        m_window,
        zh(u8"选择工程目录"),
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (projectPath.isEmpty()) {
        result.canceled = true;
        result.logMessages.append(zh(u8"已取消新建工程。"));
        return result;
    }

    Project project;
    QString errorMessage;
    if (!m_projectManager.createProject(projectPath, &project, &errorMessage)) {
        QMessageBox::warning(m_window, zh(u8"新建工程失败"), errorMessage);
        result.logMessages.append(zh(u8"新建工程失败：") + errorMessage);
        return result;
    }

    result.logMessages.append(setCurrentProject(project).logMessages);
    refreshProjectTree();
    result.logMessages.append(saveSimulationCase().logMessages);
    result.logMessages.append(zh(u8"工程已创建：") + project.rootPath);
    result.success = true;
    return result;
}

ProjectWorkflowResult ProjectWorkflowController::openProject() const
{
    ProjectWorkflowResult result;
    const QString projectFilePath = QFileDialog::getOpenFileName(
        m_window,
        zh(u8"打开工程"),
        QString(),
        "MyCAE Project (project.json);;JSON Files (*.json)"
    );

    if (projectFilePath.isEmpty()) {
        result.canceled = true;
        result.logMessages.append(zh(u8"已取消打开工程。"));
        return result;
    }

    return openProjectFile(projectFilePath);
}

ProjectWorkflowResult ProjectWorkflowController::openProjectFile(const QString &projectFilePath) const
{
    ProjectWorkflowResult result;
    Project project;
    QString errorMessage;
    if (!m_projectManager.openProject(projectFilePath, &project, &errorMessage)) {
        QMessageBox::warning(m_window, zh(u8"打开工程失败"), errorMessage);
        result.logMessages.append(zh(u8"打开工程失败：") + errorMessage);
        return result;
    }

    result.logMessages.append(setCurrentProject(project).logMessages);
    result.logMessages.append(loadGeometries().logMessages);
    result.logMessages.append(loadMeshes().logMessages);
    result.logMessages.append(loadSimulationCase().logMessages);
    result.logMessages.append(loadResults().logMessages);
    refreshProjectTree();
    result.logMessages.append(zh(u8"工程已打开：") + project.rootPath);
    result.success = true;
    return result;
}

ProjectWorkflowResult ProjectWorkflowController::setCurrentProject(const Project &project) const
{
    ProjectWorkflowResult result;
    m_projectModel.clear();
    m_projectModel.setProject(project);

    if (m_window) {
        m_window->setWindowTitle("MyCAE - " + project.name);
        if (m_window->statusBar()) {
            m_window->statusBar()->showMessage(zh(u8"当前工程：") + project.name);
        }
    }
    if (m_projectTreePanel) {
        m_projectTreePanel->showProject(project.name, project.rootPath);
    }
    if (m_propertyPanel) {
        m_propertyPanel->showEmptySelection();
    }
    if (m_renderView) {
        m_renderView->showEmpty();
    }
    result.success = true;
    return result;
}

ProjectWorkflowResult ProjectWorkflowController::loadGeometries() const
{
    ProjectWorkflowResult result;
    QString errorMessage;
    ProjectModelLoader loader(m_geometryManager);
    if (!loader.loadGeometries(m_projectModel, &errorMessage)) {
        QMessageBox::warning(m_window, zh(u8"加载几何失败"), errorMessage);
        result.logMessages.append(zh(u8"加载几何失败：") + errorMessage);
        return result;
    }

    if (m_propertyPanel) {
        m_propertyPanel->showEmptySelection();
    }
    if (m_renderView) {
        m_renderView->showEmpty();
    }
    result.logMessages.append(
        zh(u8"已加载 %1 个几何对象。").arg(m_projectModel.geometryRepository().geometryObjects().size())
    );
    result.success = true;
    return result;
}

ProjectWorkflowResult ProjectWorkflowController::loadMeshes() const
{
    ProjectWorkflowResult result;
    QString errorMessage;
    ProjectModelLoader loader(m_geometryManager);
    if (!loader.loadMeshes(m_projectModel, &errorMessage)) {
        QMessageBox::warning(m_window, zh(u8"加载网格失败"), errorMessage);
        result.logMessages.append(zh(u8"加载网格失败：") + errorMessage);
        return result;
    }

    result.logMessages.append(
        zh(u8"已加载 %1 个网格对象。").arg(m_projectModel.meshRepository().meshObjects().size())
    );
    result.success = true;
    return result;
}

ProjectWorkflowResult ProjectWorkflowController::loadSimulationCase() const
{
    ProjectWorkflowResult result;
    QString errorMessage;
    ProjectModelLoader loader(m_geometryManager);
    if (!loader.loadSimulationCase(m_projectModel, &errorMessage)) {
        QMessageBox::warning(m_window, zh(u8"加载仿真工况失败"), errorMessage);
        result.logMessages.append(zh(u8"加载仿真工况失败：") + errorMessage);
        return result;
    }

    const SolverRepository &solverRepository = m_projectModel.solverRepository();
    result.logMessages.append(zh(u8"已加载仿真工况数据：%1 个材料，%2 个材料分区，%3 个边界条件，%4 个载荷。")
        .arg(solverRepository.materials().size())
        .arg(solverRepository.sectionAssignments().size())
        .arg(solverRepository.boundaryConditions().size())
        .arg(solverRepository.loads().size()));
    result.success = true;
    return result;
}

ProjectWorkflowResult ProjectWorkflowController::loadResults() const
{
    ProjectWorkflowResult result;
    QString errorMessage;
    ProjectModelLoader loader(m_geometryManager);
    if (!loader.loadResults(m_projectModel, &errorMessage)) {
        QMessageBox::warning(m_window, zh(u8"加载结果失败"), errorMessage);
        result.logMessages.append(zh(u8"加载结果失败：") + errorMessage);
        return result;
    }

    result.logMessages.append(
        zh(u8"已加载 %1 条求解结果。").arg(m_projectModel.resultRepository().results().size())
    );
    result.success = true;
    return result;
}

ProjectWorkflowResult ProjectWorkflowController::saveSimulationCase() const
{
    ProjectWorkflowResult result;
    QString errorMessage;
    const SimulationCaseManager simulationCaseManager;
    if (!simulationCaseManager.save(m_projectModel, &errorMessage)) {
        result.logMessages.append(zh(u8"保存仿真工况失败：") + errorMessage);
        return result;
    }

    result.logMessages.append(zh(u8"仿真工况已保存：") + SimulationCaseManager::relativeCaseFilePath());
    result.success = true;
    return result;
}

void ProjectWorkflowController::refreshProjectTree() const
{
    refreshGeometryTree();
    refreshFaceGroupTree();
    refreshMeshTree();
    refreshSolverDataTree();
    refreshResultTree();
}

void ProjectWorkflowController::refreshGeometryTree() const
{
    if (!m_projectTreePanel) {
        return;
    }

    m_projectTreePanel->setGeometryItems(m_projectModel.geometryRepository().geometryObjects());
}

void ProjectWorkflowController::refreshMeshTree() const
{
    if (!m_projectTreePanel) {
        return;
    }

    QStringList meshNames;
    for (const MeshObject &meshObject : m_projectModel.meshRepository().meshObjects()) {
        meshNames.append(meshObject.name);
    }
    m_projectTreePanel->setMeshItems(meshNames);
}

void ProjectWorkflowController::refreshFaceGroupTree() const
{
    if (!m_projectTreePanel) {
        return;
    }

    m_projectTreePanel->setFaceGroupItems(m_projectModel.solverRepository().faceGroups());
}

void ProjectWorkflowController::refreshSolverDataTree() const
{
    if (!m_projectTreePanel) {
        return;
    }

    const SolverRepository &solverRepository = m_projectModel.solverRepository();
    m_projectTreePanel->setMaterialItems(solverRepository.materials());
    m_projectTreePanel->setSectionAssignmentItems(solverRepository.sectionAssignments());
    m_projectTreePanel->setBoundaryConditionItems(solverRepository.boundaryConditions());
    m_projectTreePanel->setLoadItems(solverRepository.loads());
}

void ProjectWorkflowController::refreshResultTree() const
{
    if (!m_projectTreePanel) {
        return;
    }

    m_projectTreePanel->setResultItems(m_projectModel.resultRepository().results());
}
