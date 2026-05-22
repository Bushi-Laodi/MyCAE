#include "MainWindow.h"

#include "geometry/GeometryCreationController.h"
#include "geometry/GeometryDisplayController.h"
#include "geometry/GeometryPropertyController.h"
#include "LogPanel.h"
#include "mesh/GmshRunner.h"
#include "mesh/MeshManager.h"
#include "mesh/MeshToVtkConverter.h"
#include "mesh/MshReader.h"
#include "ProjectTreePanel.h"
#include "PropertyPanel.h"
#include "project/ProjectModelLoader.h"
#include "RenderView.h"

#include <vtkUnstructuredGrid.h>

#include <QAction>
#include <QDateTime>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QStringList>
#include <QToolBar>

namespace
{
QString makeSafeFileBaseName(const QString &name)
{
    QString result = name.toLower();
    for (QChar &ch : result) {
        if (ch.isSpace()) {
            ch = '_';
        } else if (!ch.isLetterOrNumber() && ch != '_') {
            ch = '_';
        }
    }
    return result.isEmpty() ? QString("geometry") : result;
}
}

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

    statusBar()->showMessage("Ready");
    writeLog("MyCAE started.");
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

    auto *meshMenu = menuBar()->addMenu("Mesh");
    meshMenu->addAction(m_checkGmshAction);
    meshMenu->addAction(m_generateMeshAction);
    meshMenu->addAction(m_readMeshInfoAction);
    meshMenu->addAction(m_showMeshAction);

    auto *simulationMenu = menuBar()->addMenu("Simulation");
    simulationMenu->addAction("Generate Mesh", this, [this]() {
        writeLog("Use Mesh -> Generate Mesh for the current external Gmsh flow.");
    });
    simulationMenu->addAction("Run Solver", this, [this]() {
        writeLog("Solver integration is reserved for a later stage.");
    });
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
}

void MainWindow::checkGmsh()
{
    const GmshRunner gmshRunner;
    const GmshRunResult result = gmshRunner.checkVersion();

    writeLog("Gmsh path: " + gmshRunner.gmshExecutablePath());
    writeLog("Gmsh command: " + gmshRunner.gmshExecutablePath() + " --version");
    writeLog(QString("Gmsh exitCode: %1").arg(result.exitCode));
    writeLog("Gmsh stdout: " + (result.standardOutput.isEmpty() ? QString("<empty>") : result.standardOutput));
    writeLog("Gmsh stderr: " + (result.standardError.isEmpty() ? QString("<empty>") : result.standardError));

    if (result.success) {
        writeLog("Gmsh environment check succeeded.");
    } else {
        writeLog(result.errorMessage.isEmpty() ? "Gmsh environment check failed: unknown error." : result.errorMessage);
    }
}

void MainWindow::generateMesh()
{
    if (!m_projectModel.hasProject()) {
        writeLog("Generate mesh failed: create or open a project first.");
        return;
    }

    const GeometryObject *selectedGeometry = m_projectModel.selectedGeometry();
    if (!selectedGeometry) {
        writeLog("Please select a geometry object in the project tree first.");
        return;
    }

    const GeometryObject &geometry = *selectedGeometry;
    if (geometry.stepFile.isEmpty()) {
        writeLog("Current geometry has no STEP file.");
        return;
    }

    const QString stepAbsPath = QFileInfo(geometry.stepFile).isAbsolute()
        ? geometry.stepFile
        : QDir(m_projectModel.project().rootPath).filePath(geometry.stepFile);
    if (!QFileInfo::exists(stepAbsPath)) {
        writeLog("Generate mesh failed: STEP file does not exist: " + stepAbsPath);
        return;
    }

    const QString safeGeometryName = makeSafeFileBaseName(geometry.name);
    const QString meshRelativePath = QDir("mesh").filePath(safeGeometryName + ".msh");
    const QString meshAbsPath = QDir(m_projectModel.project().rootPath).filePath(meshRelativePath);

    const GmshRunner gmshRunner;
    const GmshRunResult result = gmshRunner.generate3DMesh(stepAbsPath, meshAbsPath);

    writeLog("Geometry name: " + geometry.name);
    writeLog("Geometry type: " + geometry.type);
    writeLog("Gmsh path: " + gmshRunner.gmshExecutablePath());
    writeLog("Gmsh command: " + gmshRunner.gmshExecutablePath()
             + " " + stepAbsPath
             + " -3 -format msh2 -o " + meshAbsPath);
    writeLog("Gmsh input: " + stepAbsPath);
    writeLog("Gmsh output: " + meshAbsPath);
    writeLog(QString("Gmsh exitCode: %1").arg(result.exitCode));
    writeLog("Gmsh stdout: " + (result.standardOutput.isEmpty() ? QString("<empty>") : result.standardOutput));
    writeLog("Gmsh stderr: " + (result.standardError.isEmpty() ? QString("<empty>") : result.standardError));

    if (!result.success) {
        writeLog(result.errorMessage.isEmpty() ? "Generate mesh failed: unknown error." : result.errorMessage);
        return;
    }

    writeLog("Mesh generated: " + meshRelativePath);

    MeshData meshData;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        writeLog("MeshObject save failed: cannot read generated MSH: " + errorMessage);
        return;
    }

    MeshObject meshObject;
    meshObject.name = geometry.name + "_Mesh";
    meshObject.sourceGeometryName = geometry.name;
    meshObject.sourceGeometryType = geometry.type;
    meshObject.sourceStepFile = geometry.stepFile;
    meshObject.mshFile = meshRelativePath;
    meshObject.type = "tetra4";
    meshObject.nodeCount = meshData.nodeCount();
    meshObject.tetraCount = meshData.tetraCount();
    meshObject.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);

    MeshManager meshManager(m_projectModel.project().rootPath);
    if (!meshManager.saveMeshObject(meshObject, &errorMessage)) {
        writeLog("MeshObject save failed: " + errorMessage);
        return;
    }

    bool replaced = false;
    QVector<MeshObject> &meshObjects = m_projectModel.meshObjects();
    for (int meshIndex = 0; meshIndex < meshObjects.size(); ++meshIndex) {
        MeshObject &existingMesh = meshObjects[meshIndex];
        if (existingMesh.sourceGeometryName == meshObject.sourceGeometryName) {
            existingMesh = meshObject;
            m_projectModel.setSelectedMeshName(meshObject.name);
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        meshObjects.append(meshObject);
        m_projectModel.setSelectedMeshName(meshObject.name);
    }

    refreshMeshTree();
    writeLog("MeshObject saved: mesh/" + safeGeometryName + "_mesh.json");
}

void MainWindow::readMeshInfo()
{
    if (!m_projectModel.hasProject()) {
        writeLog("Read mesh failed: create or open a project first.");
        return;
    }

    const GeometryObject *selectedGeometry = m_projectModel.selectedGeometry();
    if (!selectedGeometry) {
        writeLog("Please select a geometry object in the project tree first.");
        return;
    }

    const GeometryObject &geometry = *selectedGeometry;
    const QString meshRelativePath = QDir("mesh").filePath(makeSafeFileBaseName(geometry.name) + ".msh");
    const QString meshAbsPath = QDir(m_projectModel.project().rootPath).filePath(meshRelativePath);

    writeLog("MSH file: " + meshAbsPath);
    if (!QFileInfo::exists(meshAbsPath)) {
        writeLog("Read mesh failed: MSH file does not exist.");
        return;
    }

    MeshData meshData;
    meshData.sourceGeometryName = geometry.name;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        writeLog("Read mesh failed: " + errorMessage);
        return;
    }

    writeLog("Read mesh succeeded.");
    writeLog(QString("Node count: %1").arg(meshData.nodeCount()));
    writeLog(QString("Tetra count: %1").arg(meshData.tetraCount()));
}

void MainWindow::showMesh()
{
    if (!m_projectModel.hasProject()) {
        writeLog("Show mesh failed: create or open a project first.");
        return;
    }

    const GeometryObject *selectedGeometry = m_projectModel.selectedGeometry();
    if (!selectedGeometry) {
        writeLog("Please select a geometry object in the project tree first.");
        return;
    }

    const GeometryObject &geometry = *selectedGeometry;
    const QString meshRelativePath = QDir("mesh").filePath(makeSafeFileBaseName(geometry.name) + ".msh");
    const QString meshAbsPath = QDir(m_projectModel.project().rootPath).filePath(meshRelativePath);

    writeLog("MSH file: " + meshAbsPath);
    if (!QFileInfo::exists(meshAbsPath)) {
        writeLog("Show mesh failed: MSH file does not exist.");
        return;
    }

    MeshData meshData;
    meshData.sourceGeometryName = geometry.name;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        writeLog("Show mesh failed: cannot read MSH: " + errorMessage);
        return;
    }

    vtkSmartPointer<vtkUnstructuredGrid> grid = MeshToVtkConverter::toUnstructuredGrid(meshData, &errorMessage);
    if (!grid) {
        writeLog("VTK mesh conversion failed: " + errorMessage);
        return;
    }

    const QString subtitle = QString("%1 nodes, %2 tetrahedra")
        .arg(meshData.nodeCount())
        .arg(meshData.tetraCount());
    m_renderView->showMeshGrid(grid, geometry.name + " Mesh", subtitle);
    writeLog("Mesh displayed.");
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
    writeLog(QString("Loaded %1 boxes and %2 cylinders.")
        .arg(m_projectModel.boxes().size())
        .arg(m_projectModel.cylinders().size()));
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

    if (!m_renderView) {
        return;
    }

    const QString meshAbsPath = QFileInfo(meshObject.mshFile).isAbsolute()
        ? meshObject.mshFile
        : QDir(m_projectModel.project().rootPath).filePath(meshObject.mshFile);

    if (!QFileInfo::exists(meshAbsPath)) {
        writeLog("Show mesh failed: MSH file does not exist: " + meshObject.mshFile);
        return;
    }

    MeshData meshData;
    meshData.sourceGeometryName = meshObject.sourceGeometryName;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        writeLog("Show mesh failed: cannot read MSH: " + errorMessage);
        return;
    }

    vtkSmartPointer<vtkUnstructuredGrid> grid = MeshToVtkConverter::toUnstructuredGrid(meshData, &errorMessage);
    if (!grid) {
        writeLog("VTK mesh conversion failed: " + errorMessage);
        return;
    }

    const QString subtitle = QString("%1 nodes, %2 tetrahedra")
        .arg(meshData.nodeCount())
        .arg(meshData.tetraCount());
    m_renderView->showMeshGrid(grid, meshObject.name, subtitle);
    writeLog("Mesh displayed: " + meshObject.name);
}

void MainWindow::writeLog(const QString &message)
{
    if (m_logPanel) {
        m_logPanel->appendMessage(message);
    }
}
