#include "MainWindow.h"

#include "geometry/GeometryCreationController.h"
#include "geometry/GeometryDisplayController.h"
#include "geometry/GeometryPropertyController.h"
#include "LogPanel.h"
#include "mesh/MeshWorkflowController.h"
#include "ProjectTreePanel.h"
#include "PropertyPanel.h"
#include "project/ProjectModelLoader.h"
#include "RenderView.h"
#include "solver/SimulationCaseBuilder.h"
#include "solver/SimulationCaseManager.h"
#include "SolverDataController.h"

#include <QAction>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QShowEvent>
#include <QStatusBar>
#include <QStringList>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1200, 760);
    setWindowTitle("MyCAE - Qt 6");

    createActions();
    createMenus();
    createToolBar();

    m_renderView = new RenderView(this);
    setCentralWidget(m_renderView);
    createDockWidgets();

    // NOTE: Do NOT call m_renderView->showEmpty() here!
    // The VTK OpenGL context is not fully ready during widget construction.
    // The first Render() call is deferred to showEvent() to avoid a crash
    // in MSVCP140.dll (0xc0000005 - access violation).

    statusBar()->showMessage("Ready");
    writeLog("MyCAE started.");
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    if (!m_vtkInitialized) {
        initializeVtkRender();
    }
}

void MainWindow::initializeVtkRender()
{
    // Defer the first VTK Render() call until the window is actually shown.
    // The OpenGL context is guaranteed to be ready at this point.
    if (m_renderView) {
        m_renderView->showEmpty();
    }
    m_vtkInitialized = true;
    writeLog("VTK renderer initialized.");
}

void MainWindow::createActions()
{
    m_newProjectAction = new QAction("New Project", this);
    m_newProjectAction->setStatusTip("Create a new CAE project");
    connect(m_newProjectAction, &QAction::triggered, this, &MainWindow::newProject);

    m_openProjectAction = new QAction("Open Project", this);
    m_openProjectAction->setStatusTip("Open an existing CAE project");
    connect(m_openProjectAction, &QAction::triggered, this, &MainWindow::openProject);

    m_createBoxAction = new QAction("Create Box", this);
    m_createBoxAction->setStatusTip("Create a box geometry from length, width, and height");
    connect(m_createBoxAction, &QAction::triggered, this, [this]() {
        createGeometry(GeometryCreateType::Box);
    });

    m_createCylinderAction = new QAction("Create Cylinder", this);
    m_createCylinderAction->setStatusTip("Create a cylinder geometry from radius and height");
    connect(m_createCylinderAction, &QAction::triggered, this, [this]() {
        createGeometry(GeometryCreateType::Cylinder);
    });

    m_checkGmshAction = new QAction("Check Gmsh", this);
    m_checkGmshAction->setStatusTip("Run gmsh.exe --version");
    connect(m_checkGmshAction, &QAction::triggered, this, &MainWindow::checkGmsh);

    m_generateMeshAction = new QAction("Generate Mesh", this);
    m_generateMeshAction->setStatusTip("Generate a 3D mesh from the selected STEP geometry");
    connect(m_generateMeshAction, &QAction::triggered, this, &MainWindow::generateMesh);

    m_readMeshInfoAction = new QAction("Read Mesh Info", this);
    m_readMeshInfoAction->setStatusTip("Read node and tetrahedron counts from the selected MSH file");
    connect(m_readMeshInfoAction, &QAction::triggered, this, &MainWindow::readMeshInfo);

    m_showMeshAction = new QAction("Show Mesh", this);
    m_showMeshAction->setStatusTip("Read and display the selected tetrahedral MSH file");
    connect(m_showMeshAction, &QAction::triggered, this, &MainWindow::showMesh);

    m_exitAction = new QAction("Exit", this);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    // Solver data actions
    m_createMaterialAction = new QAction("Create Material", this);
    m_createMaterialAction->setStatusTip("Create a new material");
    connect(m_createMaterialAction, &QAction::triggered, this, &MainWindow::createMaterial);

    m_createBoundaryConditionAction = new QAction("Create Boundary Condition", this);
    m_createBoundaryConditionAction->setStatusTip("Create a new boundary condition");
    connect(m_createBoundaryConditionAction, &QAction::triggered, this, &MainWindow::createBoundaryCondition);

    m_createLoadAction = new QAction("Create Load", this);
    m_createLoadAction->setStatusTip("Create a new load");
    connect(m_createLoadAction, &QAction::triggered, this, &MainWindow::createLoad);

    m_editSolverDataAction = new QAction("Edit Selected Solver Data", this);
    m_editSolverDataAction->setStatusTip("Edit the selected material, boundary condition, or load");
    connect(m_editSolverDataAction, &QAction::triggered, this, &MainWindow::editSelectedSolverData);

    m_deleteSolverDataAction = new QAction("Delete Selected Solver Data", this);
    m_deleteSolverDataAction->setStatusTip("Delete the selected material, boundary condition, or load");
    connect(m_deleteSolverDataAction, &QAction::triggered, this, &MainWindow::deleteSelectedSolverData);
}

void MainWindow::createMenus()
{
    auto *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(m_newProjectAction);
    fileMenu->addAction(m_openProjectAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    auto *geometryMenu = menuBar()->addMenu("Geometry");
    geometryMenu->addAction(m_createBoxAction);
    geometryMenu->addAction(m_createCylinderAction);
    geometryMenu->addAction("Import STEP", this, [this]() {
        writeLog("STEP import is reserved for a later stage.");
    });

    auto *solverMenu = menuBar()->addMenu("Solver Setup");
    solverMenu->addAction(m_createMaterialAction);
    solverMenu->addAction(m_createBoundaryConditionAction);
    solverMenu->addAction(m_createLoadAction);
    solverMenu->addSeparator();
    solverMenu->addAction(m_editSolverDataAction);
    solverMenu->addAction(m_deleteSolverDataAction);

    auto *meshMenu = menuBar()->addMenu("Mesh");
    meshMenu->addAction(m_checkGmshAction);
    meshMenu->addAction(m_generateMeshAction);
    meshMenu->addAction(m_readMeshInfoAction);
    meshMenu->addAction(m_showMeshAction);

    auto *simulationMenu = menuBar()->addMenu("Simulation");
    simulationMenu->addAction("Generate Mesh", this, [this]() {
        writeLog("Use Mesh -> Generate Mesh for the current external Gmsh flow.");
    });

    simulationMenu->addSeparator();
    if (m_solverPluginManager.plugins().empty()) {
        QAction *noSolverAction = simulationMenu->addAction("No solver plugins found");
        noSolverAction->setEnabled(false);
    } else {
        for (const std::unique_ptr<SolverPlugin> &plugin : m_solverPluginManager.plugins()) {
            const QString pluginId = plugin->id();
            QAction *runSolverAction = simulationMenu->addAction("Run " + plugin->name());
            runSolverAction->setStatusTip("Run solver plugin: " + pluginId);
            connect(runSolverAction, &QAction::triggered, this, [this, pluginId]() {
                runSolverPlugin(pluginId);
            });
        }
    }
}

void MainWindow::createToolBar()
{
    auto *toolBar = addToolBar("Main Toolbar");
    toolBar->setMovable(false);
    toolBar->addAction(m_newProjectAction);
    toolBar->addAction(m_openProjectAction);
    toolBar->addSeparator();
    toolBar->addAction(m_createBoxAction);
    toolBar->addAction(m_createCylinderAction);
}

void MainWindow::createDockWidgets()
{
    auto *projectDock = new QDockWidget("Project / Model", this);
    m_projectTreePanel = new ProjectTreePanel(projectDock);
    connect(m_projectTreePanel, &ProjectTreePanel::geometrySelected, this, &MainWindow::showGeometryProperties);
    connect(m_projectTreePanel, &ProjectTreePanel::meshSelected, this, &MainWindow::showMeshObject);
    connect(m_projectTreePanel, &ProjectTreePanel::materialSelected, this, &MainWindow::showMaterial);
    connect(m_projectTreePanel, &ProjectTreePanel::boundaryConditionSelected, this, &MainWindow::showBoundaryCondition);
    connect(m_projectTreePanel, &ProjectTreePanel::loadSelected, this, &MainWindow::showLoad);
    connect(m_projectTreePanel, &ProjectTreePanel::materialCategorySelected, this, &MainWindow::onMaterialCategorySelected);
    connect(m_projectTreePanel, &ProjectTreePanel::boundaryConditionCategorySelected, this, &MainWindow::onBoundaryConditionCategorySelected);
    connect(m_projectTreePanel, &ProjectTreePanel::loadCategorySelected, this, &MainWindow::onLoadCategorySelected);
    connect(m_projectTreePanel, &ProjectTreePanel::solverCategorySelected, this, &MainWindow::onSolverCategorySelected);
    projectDock->setWidget(m_projectTreePanel);
    addDockWidget(Qt::LeftDockWidgetArea, projectDock);

    auto *propertyDock = new QDockWidget("Properties", this);
    m_propertyPanel = new PropertyPanel(propertyDock);
    propertyDock->setWidget(m_propertyPanel);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock);

    auto *logDock = new QDockWidget("Log", this);
    m_logPanel = new LogPanel(logDock);
    logDock->setWidget(m_logPanel);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
}

void MainWindow::newProject()
{
    const QString projectPath = QFileDialog::getExistingDirectory(
        this,
        "Select Project Directory",
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (projectPath.isEmpty()) {
        writeLog("New project canceled.");
        return;
    }

    Project project;
    QString errorMessage;
    if (!m_projectManager.createProject(projectPath, &project, &errorMessage)) {
        QMessageBox::warning(this, "New project failed", errorMessage);
        writeLog("New project failed: " + errorMessage);
        return;
    }

    setCurrentProject(project);
    refreshGeometryTree();
    refreshMeshTree();
    refreshSolverDataTree();
    saveProjectSimulationCase();
    writeLog("Project created: " + project.rootPath);
}

void MainWindow::openProject()
{
    const QString projectFilePath = QFileDialog::getOpenFileName(
        this,
        "Open Project",
        QString(),
        "MyCAE Project (project.json);;JSON Files (*.json)"
    );

    if (projectFilePath.isEmpty()) {
        writeLog("Open project canceled.");
        return;
    }

    Project project;
    QString errorMessage;
    if (!m_projectManager.openProject(projectFilePath, &project, &errorMessage)) {
        QMessageBox::warning(this, "Open project failed", errorMessage);
        writeLog("Open project failed: " + errorMessage);
        return;
    }

    setCurrentProject(project);
    loadProjectGeometries();
    loadProjectMeshes();
    loadProjectSimulationCase();
    refreshSolverDataTree();
    writeLog("Project opened: " + project.rootPath);
}

void MainWindow::createGeometry(GeometryCreateType type)
{
    GeometryCreationController creationController(m_geometryManager);
    const GeometryCreationResult result = creationController.createGeometry(this, m_projectModel.project(), type);

    for (const QString &message : result.logMessages) {
        writeLog(message);
    }

    if (result.canceled) {
        return;
    }
    if (!result.success) {
        QMessageBox::warning(this, "Create geometry failed", result.errorMessage);
        return;
    }

    const QString createdGeometryName = result.geometryObject.name;
    loadProjectGeometries();
    if (!selectGeometryByName(createdGeometryName)) {
        writeLog("Created geometry was saved but could not be selected: " + createdGeometryName);
    }
    saveProjectSimulationCase();
}

void MainWindow::checkGmsh()
{
    const MeshWorkflowController controller;
    handleMeshWorkflowResult(controller.checkGmsh());
}

void MainWindow::generateMesh()
{
    const MeshWorkflowController controller;
    handleMeshWorkflowResult(controller.generateMesh(m_projectModel));
}

void MainWindow::readMeshInfo()
{
    const MeshWorkflowController controller;
    handleMeshWorkflowResult(controller.readMeshInfo(m_projectModel));
}

void MainWindow::showMesh()
{
    const MeshWorkflowController controller;
    handleMeshWorkflowResult(controller.showSelectedGeometryMesh(m_projectModel, m_renderView));
}

void MainWindow::runSolverPlugin(const QString &pluginId)
{
    if (!m_projectModel.hasProject()) {
        writeLog("Run solver failed: create or open a project first.");
        return;
    }

    if (!saveProjectSimulationCase()) {
        return;
    }

    const SimulationCase simulationCase = SimulationCaseBuilder::fromProjectModel(m_projectModel);
    const QString caseDirectory = QDir(m_projectModel.project().rootPath).filePath("solver/" + pluginId);

    const SolverPlugin *plugin = m_solverPluginManager.pluginById(pluginId);
    if (!plugin) {
        writeLog("Run solver failed: plugin is not registered: " + pluginId);
        return;
    }

    QString errorMessage;
    writeLog("Solver plugin: " + plugin->name());
    writeLog("Solver case directory: " + caseDirectory);
    writeLog(QString("Solver case data: %1 materials, %2 boundary conditions, %3 loads.")
        .arg(simulationCase.materials.size())
        .arg(simulationCase.boundaryConditions.size())
        .arg(simulationCase.loads.size()));

    if (!plugin->exportCase(simulationCase, caseDirectory, &errorMessage)) {
        writeLog("Solver export failed: " + errorMessage);
        return;
    }
    writeLog("Solver input exported.");

    QString solverLog;
    if (!plugin->runCase(caseDirectory, &solverLog, &errorMessage)) {
        writeLog("Solver run failed: " + errorMessage);
        return;
    }
    if (!solverLog.isEmpty()) {
        writeLog(solverLog);
    }

    QString resultText;
    if (!plugin->readResult(caseDirectory, &resultText, &errorMessage)) {
        writeLog("Solver result read failed: " + errorMessage);
        return;
    }
    writeLog("Solver result: " + resultText);
}

void MainWindow::setCurrentProject(const Project &project)
{
    m_projectModel.clear();
    m_projectModel.setProject(project);
    setWindowTitle("MyCAE - " + project.name);

    if (m_projectTreePanel) {
        m_projectTreePanel->showProject(project.name, project.rootPath);
    }
    if (m_propertyPanel) {
        m_propertyPanel->showEmptySelection();
    }
    if (m_renderView) {
        m_renderView->showEmpty();
    }

    statusBar()->showMessage("Current project: " + project.name);
}

void MainWindow::loadProjectGeometries()
{
    QString errorMessage;
    ProjectModelLoader loader(m_geometryManager);
    if (!loader.loadGeometries(m_projectModel, &errorMessage)) {
        QMessageBox::warning(this, "Load geometry failed", errorMessage);
        writeLog("Load geometry failed: " + errorMessage);
        return;
    }

    refreshGeometryTree();
    if (m_propertyPanel) {
        m_propertyPanel->showEmptySelection();
    }
    if (m_renderView) {
        m_renderView->showEmpty();
    }
    writeLog(QString("Loaded %1 geometry objects.").arg(m_projectModel.geometryObjects().size()));
}

void MainWindow::loadProjectMeshes()
{
    QString errorMessage;
    ProjectModelLoader loader(m_geometryManager);
    if (!loader.loadMeshes(m_projectModel, &errorMessage)) {
        QMessageBox::warning(this, "Load mesh failed", errorMessage);
        writeLog("Load mesh failed: " + errorMessage);
        return;
    }

    refreshMeshTree();
    writeLog(QString("Loaded %1 mesh objects.").arg(m_projectModel.meshObjects().size()));
}

void MainWindow::loadProjectSimulationCase()
{
    QString errorMessage;
    ProjectModelLoader loader(m_geometryManager);
    if (!loader.loadSimulationCase(m_projectModel, &errorMessage)) {
        QMessageBox::warning(this, "Load simulation case failed", errorMessage);
        writeLog("Load simulation case failed: " + errorMessage);
        return;
    }

    writeLog(QString("Loaded simulation case data: %1 materials, %2 boundary conditions, %3 loads.")
        .arg(m_projectModel.materials().size())
        .arg(m_projectModel.boundaryConditions().size())
        .arg(m_projectModel.loads().size()));
}

bool MainWindow::saveProjectSimulationCase()
{
    QString errorMessage;
    const SimulationCaseManager simulationCaseManager;
    if (!simulationCaseManager.save(m_projectModel, &errorMessage)) {
        writeLog("Save simulation case failed: " + errorMessage);
        return false;
    }

    writeLog("Simulation case saved: " + SimulationCaseManager::relativeCaseFilePath());
    return true;
}

void MainWindow::refreshGeometryTree()
{
    if (!m_projectTreePanel) {
        return;
    }

    QStringList geometryNames;
    for (const GeometryObject &geometry : m_projectModel.geometryObjects()) {
        geometryNames.append(geometry.name);
    }
    m_projectTreePanel->setGeometryItems(geometryNames);
}

void MainWindow::refreshMeshTree()
{
    if (!m_projectTreePanel) {
        return;
    }

    QStringList meshNames;
    for (const MeshObject &meshObject : m_projectModel.meshObjects()) {
        meshNames.append(meshObject.name);
    }
    m_projectTreePanel->setMeshItems(meshNames);
}

void MainWindow::refreshSolverDataTree()
{
    if (!m_projectTreePanel) {
        return;
    }

    m_projectTreePanel->setMaterialItems(m_projectModel.materials());
    m_projectTreePanel->setBoundaryConditionItems(m_projectModel.boundaryConditions());
    m_projectTreePanel->setLoadItems(m_projectModel.loads());
}

bool MainWindow::selectGeometryByName(const QString &geometryName)
{
    showGeometryProperties(geometryName);
    return m_projectModel.selectedGeometryName() == geometryName;
}

void MainWindow::showGeometryProperties(const QString &geometryName)
{
    const GeometryObject *geometry = m_projectModel.findGeometryByName(geometryName);
    if (!geometry) {
        m_projectModel.clearSelectedGeometry();
        return;
    }

    m_projectModel.setSelectedGeometryName(geometry->name);

    const GeometryPropertyController propertyController;
    const GeometryPropertyResult propertyResult = propertyController.showGeometryProperties(
        m_projectModel,
        *geometry,
        m_propertyPanel
    );
    if (!propertyResult.success) {
        writeLog(propertyResult.errorMessage);
    }

    displayGeometry(*geometry);
}

void MainWindow::showMeshObject(const QString &meshName)
{
    const MeshObject *meshObject = m_projectModel.findMeshByName(meshName);
    if (meshObject) {
        m_projectModel.setSelectedMeshName(meshName);
        displayMeshObject(*meshObject);
        return;
    }
    m_projectModel.clearSelectedMesh();
}

void MainWindow::showMaterial(const QString &materialId)
{
    for (const QString &message : SolverDataController::showMaterial(m_projectModel, m_propertyPanel, materialId)) {
        writeLog(message);
    }
}

void MainWindow::showBoundaryCondition(const QString &boundaryConditionId)
{
    for (const QString &message :
         SolverDataController::showBoundaryCondition(m_projectModel, m_propertyPanel, boundaryConditionId)) {
        writeLog(message);
    }
}

void MainWindow::showLoad(const QString &loadId)
{
    for (const QString &message : SolverDataController::showLoad(m_projectModel, m_propertyPanel, loadId)) {
        writeLog(message);
    }
}

void MainWindow::displayGeometry(const GeometryObject &geometry)
{
    const GeometryDisplayController displayController;
    const GeometryDisplayResult result = displayController.displayGeometry(
        m_projectModel,
        geometry,
        m_renderView
    );

    for (const QString &message : result.logMessages) {
        writeLog(message);
    }
}

void MainWindow::displayMeshObject(const MeshObject &meshObject)
{
    if (m_propertyPanel) {
        m_propertyPanel->showMeshObject(meshObject);
    }

    const MeshWorkflowController controller;
    handleMeshWorkflowResult(controller.displayMeshObject(m_projectModel, meshObject, m_renderView));
}

void MainWindow::writeLog(const QString &message)
{
    if (m_logPanel) {
        m_logPanel->appendMessage(message);
    }
}

void MainWindow::handleMeshWorkflowResult(const MeshWorkflowResult &result)
{
    for (const QString &message : result.logMessages) {
        writeLog(message);
    }

    if (result.meshTreeChanged) {
        refreshMeshTree();
    }
    if (result.simulationCaseChanged) {
        saveProjectSimulationCase();
    }
}

void MainWindow::handleSolverDataResult(const SolverDataControllerResult &result)
{
    for (const QString &message : result.logMessages) {
        writeLog(message);
    }

    if (!result.changed) {
        return;
    }

    saveProjectSimulationCase();
    refreshSolverDataTree();

    switch (result.selectionKind) {
    case SolverDataSelectionKind::MaterialCategory:
        onMaterialCategorySelected();
        break;
    case SolverDataSelectionKind::BoundaryConditionCategory:
        onBoundaryConditionCategorySelected();
        break;
    case SolverDataSelectionKind::LoadCategory:
        onLoadCategorySelected();
        break;
    case SolverDataSelectionKind::Material:
        showMaterial(result.selectionId);
        break;
    case SolverDataSelectionKind::BoundaryCondition:
        showBoundaryCondition(result.selectionId);
        break;
    case SolverDataSelectionKind::Load:
        showLoad(result.selectionId);
        break;
    case SolverDataSelectionKind::None:
        break;
    }
}

// ============================================================
// Solver data UI handlers
// ============================================================

void MainWindow::onMaterialCategorySelected()
{
    for (const QString &message : SolverDataController::showMaterialCategory(m_projectModel, m_propertyPanel)) {
        writeLog(message);
    }
}

void MainWindow::onBoundaryConditionCategorySelected()
{
    for (const QString &message :
         SolverDataController::showBoundaryConditionCategory(m_projectModel, m_propertyPanel)) {
        writeLog(message);
    }
}

void MainWindow::onLoadCategorySelected()
{
    for (const QString &message : SolverDataController::showLoadCategory(m_projectModel, m_propertyPanel)) {
        writeLog(message);
    }
}

void MainWindow::onSolverCategorySelected()
{
    m_projectModel.clearSelectedGeometry();
    m_projectModel.clearSelectedMesh();
    m_projectModel.clearSelectedSolverData();
    if (m_propertyPanel) {
        m_propertyPanel->showSolverCategory(SimulationCaseBuilder::fromProjectModel(m_projectModel));
    }
    writeLog("Solver settings displayed.");
}

void MainWindow::createMaterial()
{
    handleSolverDataResult(SolverDataController::createMaterial(this, m_projectModel));
}

void MainWindow::createBoundaryCondition()
{
    handleSolverDataResult(SolverDataController::createBoundaryCondition(this, m_projectModel));
}

void MainWindow::createLoad()
{
    handleSolverDataResult(SolverDataController::createLoad(this, m_projectModel));
}

void MainWindow::editSelectedSolverData()
{
    handleSolverDataResult(SolverDataController::editSelected(this, m_projectModel));
}

void MainWindow::deleteSelectedSolverData()
{
    handleSolverDataResult(SolverDataController::deleteSelected(this, m_projectModel));
}
