#include "MainWindow.h"

#include "LogPanel.h"
#include "ProjectTreePanel.h"
#include "PropertyPanel.h"
#include "RenderView.h"
#include "commands/FaceGroupEditCommands.h"
#include "commands/GeometryCommands.h"
#include "commands/MeshCommands.h"
#include "commands/PickModeCommands.h"
#include "commands/ProjectCommands.h"
#include "commands/SolverCommands.h"
#include "commands/UtilityCommands.h"
#include "commands/WorkflowCommandContext.h"
#include "solver/plugin/SolverPluginDescriptorFormatter.h"
#include "workflow/SelectionController.h"

#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QShowEvent>
#include <QStatusBar>
#include <QToolBar>

namespace
{
constexpr auto CommandNewProject = "project.new";
constexpr auto CommandOpenProject = "project.open";
constexpr auto CommandExit = "app.exit";
constexpr auto CommandCreateBox = "geometry.create.box";
constexpr auto CommandCreateCylinder = "geometry.create.cylinder";
constexpr auto CommandImportStep = "geometry.import.step";
constexpr auto CommandCheckGmsh = "mesh.checkGmsh";
constexpr auto CommandGenerateMesh = "mesh.generate";
constexpr auto CommandReadMeshInfo = "mesh.readInfo";
constexpr auto CommandShowMesh = "mesh.show";
constexpr auto CommandPickFace = "picking.face";
constexpr auto CommandClearPick = "picking.clear";
constexpr auto CommandCreateFaceGroupFromPick = "picking.faceGroup.createFromPick";
constexpr auto CommandAddPickedFacesToFaceGroup = "picking.faceGroup.addPicked";
constexpr auto CommandRemovePickedFacesFromFaceGroup = "picking.faceGroup.removePicked";
constexpr auto CommandClearFaceGroupFaces = "picking.faceGroup.clearFaces";
constexpr auto CommandRenameFaceGroup = "picking.faceGroup.rename";
constexpr auto CommandDeleteFaceGroup = "picking.faceGroup.delete";
constexpr auto CommandSetFaceGroupLocalMeshSize = "picking.faceGroup.localMeshSize";
constexpr auto CommandToggleFaceGroupPhysicalGroup = "picking.faceGroup.togglePhysicalGroup";
constexpr auto CommandCreateMaterial = "solverData.create.material";
constexpr auto CommandCreateBoundaryCondition = "solverData.create.boundaryCondition";
constexpr auto CommandCreateLoad = "solverData.create.load";
constexpr auto CommandEditSolverData = "solverData.editSelected";
constexpr auto CommandDeleteSolverData = "solverData.deleteSelected";
constexpr auto CommandSimulationGenerateMesh = "simulation.generateMesh";

QString solverRunCommandId(const QString &pluginId)
{
    return "solver.run." + pluginId;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1200, 760);
    setWindowTitle("MyCAE - Qt 6");

    m_renderView = new RenderView(this);
    connect(m_renderView, &RenderView::facePicked, this, &MainWindow::handleFacePicked);
    setCentralWidget(m_renderView);
    createDockWidgets();

    createActions();
    createMenus();
    createToolBar();
    m_actionRegistry.setAfterExecuteCallback([this]() {
        updateActionStates();
    });
    updateActionStates();

    // NOTE: Do NOT call m_renderView->showEmpty() here!
    // The VTK OpenGL context is not fully ready during widget construction.
    // The first Render() call is deferred to showEvent() to avoid a crash
    // in MSVCP140.dll (0xc0000005 - access violation).

    statusBar()->showMessage("Ready");
    writeLog("MyCAE started.");
    writeLogMessages(m_solverPluginManager.diagnostics());
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
    const WorkflowCommandContext context = workflowCommandContext();

    m_newProjectAction = new QAction("New Project", this);
    m_newProjectAction->setStatusTip("Create a new CAE project");
    m_actionRegistry.registerActionCommand(
        CommandNewProject,
        m_newProjectAction,
        this,
        makeProjectCommand(context, ProjectCommandType::Create)
    );

    m_openProjectAction = new QAction("Open Project", this);
    m_openProjectAction->setStatusTip("Open an existing CAE project");
    m_actionRegistry.registerActionCommand(
        CommandOpenProject,
        m_openProjectAction,
        this,
        makeProjectCommand(context, ProjectCommandType::Open)
    );

    m_createBoxAction = new QAction("Create Box", this);
    m_createBoxAction->setStatusTip("Create a box geometry from length, width, and height");
    m_actionRegistry.registerActionCommand(
        CommandCreateBox,
        m_createBoxAction,
        this,
        makeGeometryCreateCommand(context, GeometryCreateType::Box)
    );

    m_createCylinderAction = new QAction("Create Cylinder", this);
    m_createCylinderAction->setStatusTip("Create a cylinder geometry from radius and height");
    m_actionRegistry.registerActionCommand(
        CommandCreateCylinder,
        m_createCylinderAction,
        this,
        makeGeometryCreateCommand(context, GeometryCreateType::Cylinder)
    );

    m_checkGmshAction = new QAction("Check Gmsh", this);
    m_checkGmshAction->setStatusTip("Run gmsh.exe --version");
    m_actionRegistry.registerActionCommand(
        CommandCheckGmsh,
        m_checkGmshAction,
        this,
        makeMeshCommand(context, MeshCommandType::CheckGmsh)
    );

    m_generateMeshAction = new QAction("Generate Mesh", this);
    m_generateMeshAction->setStatusTip("Generate a 3D mesh from the selected STEP geometry");
    m_actionRegistry.registerActionCommand(
        CommandGenerateMesh,
        m_generateMeshAction,
        this,
        makeMeshCommand(context, MeshCommandType::Generate)
    );

    m_readMeshInfoAction = new QAction("Read Mesh Info", this);
    m_readMeshInfoAction->setStatusTip("Read node and tetrahedron counts from the selected MSH file");
    m_actionRegistry.registerActionCommand(
        CommandReadMeshInfo,
        m_readMeshInfoAction,
        this,
        makeMeshCommand(context, MeshCommandType::ReadInfo)
    );

    m_showMeshAction = new QAction("Show Mesh", this);
    m_showMeshAction->setStatusTip("Read and display the selected tetrahedral MSH file");
    m_actionRegistry.registerActionCommand(
        CommandShowMesh,
        m_showMeshAction,
        this,
        makeMeshCommand(context, MeshCommandType::Show)
    );

    m_pickFaceAction = new QAction("Pick Face", this);
    m_pickFaceAction->setCheckable(true);
    m_pickFaceAction->setStatusTip("Pick geometry faces in the render view");
    m_actionRegistry.registerActionCommand(
        CommandPickFace,
        m_pickFaceAction,
        this,
        makePickModeCommand(context)
    );

    m_clearPickAction = new QAction("Clear Pick", this);
    m_clearPickAction->setStatusTip("Clear the current pick highlight");
    m_actionRegistry.registerActionCommand(
        CommandClearPick,
        m_clearPickAction,
        this,
        makeClearPickCommand(context)
    );

    m_createFaceGroupFromPickAction = new QAction("Create Face Group from Pick", this);
    m_createFaceGroupFromPickAction->setStatusTip("Create or update a face group from the picked face");
    m_actionRegistry.registerActionCommand(
        CommandCreateFaceGroupFromPick,
        m_createFaceGroupFromPickAction,
        this,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::CreateFromPick)
    );

    m_addPickedFacesToFaceGroupAction = new QAction("Add Picked Faces to Face Group", this);
    m_actionRegistry.registerActionCommand(
        CommandAddPickedFacesToFaceGroup,
        m_addPickedFacesToFaceGroupAction,
        this,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::AddPickedFaces)
    );

    m_removePickedFacesFromFaceGroupAction = new QAction("Remove Picked Faces from Face Group", this);
    m_actionRegistry.registerActionCommand(
        CommandRemovePickedFacesFromFaceGroup,
        m_removePickedFacesFromFaceGroupAction,
        this,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::RemovePickedFaces)
    );

    m_clearFaceGroupFacesAction = new QAction("Clear Selected Face Group Faces", this);
    m_actionRegistry.registerActionCommand(
        CommandClearFaceGroupFaces,
        m_clearFaceGroupFacesAction,
        this,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::ClearFaces)
    );

    m_renameFaceGroupAction = new QAction("Rename Selected Face Group", this);
    m_actionRegistry.registerActionCommand(
        CommandRenameFaceGroup,
        m_renameFaceGroupAction,
        this,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::Rename)
    );

    m_deleteFaceGroupAction = new QAction("Delete Selected Face Group", this);
    m_actionRegistry.registerActionCommand(
        CommandDeleteFaceGroup,
        m_deleteFaceGroupAction,
        this,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::Delete)
    );

    m_setFaceGroupLocalMeshSizeAction = new QAction("Set Face Group Local Mesh Size", this);
    m_actionRegistry.registerActionCommand(
        CommandSetFaceGroupLocalMeshSize,
        m_setFaceGroupLocalMeshSizeAction,
        this,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::SetLocalMeshSize)
    );

    m_toggleFaceGroupPhysicalGroupAction = new QAction("Toggle Face Group Physical Group", this);
    m_actionRegistry.registerActionCommand(
        CommandToggleFaceGroupPhysicalGroup,
        m_toggleFaceGroupPhysicalGroupAction,
        this,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::TogglePhysicalGroup)
    );

    m_exitAction = new QAction("Exit", this);
    m_actionRegistry.registerActionCommand(CommandExit, m_exitAction, this, makeCloseWindowCommand(this));

    m_createMaterialAction = new QAction("Create Material", this);
    m_createMaterialAction->setStatusTip("Create a new material");
    m_actionRegistry.registerActionCommand(
        CommandCreateMaterial,
        m_createMaterialAction,
        this,
        makeSolverDataCommand(context, SolverDataCommandType::CreateMaterial)
    );

    m_createBoundaryConditionAction = new QAction("Create Boundary Condition", this);
    m_createBoundaryConditionAction->setStatusTip("Create a new boundary condition");
    m_actionRegistry.registerActionCommand(
        CommandCreateBoundaryCondition,
        m_createBoundaryConditionAction,
        this,
        makeSolverDataCommand(context, SolverDataCommandType::CreateBoundaryCondition)
    );

    m_createLoadAction = new QAction("Create Load", this);
    m_createLoadAction->setStatusTip("Create a new load");
    m_actionRegistry.registerActionCommand(
        CommandCreateLoad,
        m_createLoadAction,
        this,
        makeSolverDataCommand(context, SolverDataCommandType::CreateLoad)
    );

    m_editSolverDataAction = new QAction("Edit Selected Solver Data", this);
    m_editSolverDataAction->setStatusTip("Edit the selected material, boundary condition, or load");
    m_actionRegistry.registerActionCommand(
        CommandEditSolverData,
        m_editSolverDataAction,
        this,
        makeSolverDataCommand(context, SolverDataCommandType::EditSelected)
    );

    m_deleteSolverDataAction = new QAction("Delete Selected Solver Data", this);
    m_deleteSolverDataAction->setStatusTip("Delete the selected material, boundary condition, or load");
    m_actionRegistry.registerActionCommand(
        CommandDeleteSolverData,
        m_deleteSolverDataAction,
        this,
        makeSolverDataCommand(context, SolverDataCommandType::DeleteSelected)
    );
}

void MainWindow::createMenus()
{
    const WorkflowCommandContext context = workflowCommandContext();

    auto *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(m_newProjectAction);
    fileMenu->addAction(m_openProjectAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    auto *geometryMenu = menuBar()->addMenu("Geometry");
    geometryMenu->addAction(m_createBoxAction);
    geometryMenu->addAction(m_createCylinderAction);
    QAction *importStepAction = geometryMenu->addAction("Import STEP");
    m_actionRegistry.registerActionCommand(
        CommandImportStep,
        importStepAction,
        this,
        makeLogMessageCommand(m_logPanel, "STEP import is reserved for a later stage.")
    );

    auto *pickingMenu = menuBar()->addMenu("Picking");
    pickingMenu->addAction(m_pickFaceAction);
    pickingMenu->addAction(m_clearPickAction);
    pickingMenu->addSeparator();
    pickingMenu->addAction(m_createFaceGroupFromPickAction);
    pickingMenu->addAction(m_addPickedFacesToFaceGroupAction);
    pickingMenu->addAction(m_removePickedFacesFromFaceGroupAction);
    pickingMenu->addAction(m_clearFaceGroupFacesAction);
    pickingMenu->addSeparator();
    pickingMenu->addAction(m_renameFaceGroupAction);
    pickingMenu->addAction(m_deleteFaceGroupAction);
    pickingMenu->addSeparator();
    pickingMenu->addAction(m_setFaceGroupLocalMeshSizeAction);
    pickingMenu->addAction(m_toggleFaceGroupPhysicalGroupAction);

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
    QAction *simulationGenerateMeshAction = simulationMenu->addAction("Generate Mesh");
    m_actionRegistry.registerActionCommand(
        CommandSimulationGenerateMesh,
        simulationGenerateMeshAction,
        this,
        makeLogMessageCommand(m_logPanel, "Use Mesh -> Generate Mesh for the current external Gmsh flow.")
    );

    simulationMenu->addSeparator();
    if (m_solverPluginManager.pluginDescriptors().empty()) {
        QAction *noSolverAction = simulationMenu->addAction("No solver plugins found");
        noSolverAction->setEnabled(false);
    } else {
        for (const SolverPluginDescriptor &descriptor : m_solverPluginManager.pluginDescriptors()) {
            const QString pluginId = descriptor.id;
            QAction *runSolverAction =
                simulationMenu->addAction(SolverPluginDescriptorFormatter::menuText(descriptor));
            runSolverAction->setProperty("solverUsable", descriptor.isUsable());
            m_runSolverActions.append(runSolverAction);
            runSolverAction->setStatusTip(SolverPluginDescriptorFormatter::statusTip(descriptor));
            m_actionRegistry.registerActionCommand(
                solverRunCommandId(pluginId),
                runSolverAction,
                this,
                makeRunSolverCommand(context, pluginId)
            );
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
    toolBar->addSeparator();
    toolBar->addAction(m_pickFaceAction);
    toolBar->addAction(m_createFaceGroupFromPickAction);
    toolBar->addAction(m_addPickedFacesToFaceGroupAction);
    toolBar->addAction(m_removePickedFacesFromFaceGroupAction);
}

void MainWindow::createDockWidgets()
{
    auto *projectDock = new QDockWidget("Project / Model", this);
    m_projectTreePanel = new ProjectTreePanel(projectDock);
    connect(m_projectTreePanel, &ProjectTreePanel::selectionChanged, this, &MainWindow::applySelection);
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

void MainWindow::applySelection(const Selection &selection)
{
    const SelectionController controller(m_projectModel, m_propertyPanel, m_renderView);
    if (selection.kind == SelectionKind::Geometry) {
        m_pickController.setTargetGeometry(selection.id);
    } else if (selection.kind == SelectionKind::FaceGroup) {
        if (const FaceGroup *faceGroup = m_projectModel.findFaceGroupById(selection.id)) {
            m_pickController.setTargetGeometry(faceGroup->geometryName);
        }
    }
    const SelectionControllerResult result = controller.apply(selection);
    writeLogMessages(result.logMessages);
    if (selection.kind != SelectionKind::FaceGroup) {
        m_pickController.clear(m_renderView);
    }
    updateActionStates();
}

void MainWindow::handleFacePicked(const PickSelection &selection)
{
    const PickControllerResult result = m_pickController.acceptSelection(selection, m_renderView);
    writeLogMessages(result.logMessages);
    if (m_propertyPanel && result.success) {
        m_propertyPanel->showPickState(
            m_pickController.mode(),
            m_pickController.geometryName(),
            m_pickController.selectedFaceIndices()
        );
    }
    if (result.success) {
        statusBar()->showMessage(
            QString("Pick: %1 face(s) on %2")
                .arg(result.selectedFaceCount)
                .arg(result.geometryName)
        );
    }
    updateActionStates();
}

void MainWindow::updateActionStates()
{
    const bool hasProject = m_projectModel.hasProject();
    const SelectionCapabilities capabilities = m_projectModel.selectionCapabilities();
    const bool hasPickedFaces = m_pickController.hasSelection();
    const bool hasSelectedFaceGroup = m_projectModel.selection().kind == SelectionKind::FaceGroup;

    if (m_createBoxAction) {
        m_createBoxAction->setEnabled(hasProject);
    }
    if (m_createCylinderAction) {
        m_createCylinderAction->setEnabled(hasProject);
    }
    if (m_generateMeshAction) {
        m_generateMeshAction->setEnabled(hasProject && capabilities.canGenerateMesh);
    }
    if (m_readMeshInfoAction) {
        m_readMeshInfoAction->setEnabled(hasProject && capabilities.canReadMeshInfo);
    }
    if (m_showMeshAction) {
        m_showMeshAction->setEnabled(hasProject && capabilities.canShowMesh);
    }
    if (m_pickFaceAction) {
        m_pickFaceAction->setEnabled(hasProject);
        m_pickFaceAction->setChecked(m_pickController.mode() == PickMode::Face);
    }
    if (m_clearPickAction) {
        m_clearPickAction->setEnabled(hasProject);
    }
    if (m_createFaceGroupFromPickAction) {
        m_createFaceGroupFromPickAction->setEnabled(hasProject && hasPickedFaces);
    }
    if (m_addPickedFacesToFaceGroupAction) {
        m_addPickedFacesToFaceGroupAction->setEnabled(hasProject && hasPickedFaces && hasSelectedFaceGroup);
    }
    if (m_removePickedFacesFromFaceGroupAction) {
        m_removePickedFacesFromFaceGroupAction->setEnabled(hasProject && hasPickedFaces && hasSelectedFaceGroup);
    }
    if (m_clearFaceGroupFacesAction) {
        m_clearFaceGroupFacesAction->setEnabled(hasProject && hasSelectedFaceGroup);
    }
    if (m_renameFaceGroupAction) {
        m_renameFaceGroupAction->setEnabled(hasProject && hasSelectedFaceGroup);
    }
    if (m_deleteFaceGroupAction) {
        m_deleteFaceGroupAction->setEnabled(hasProject && hasSelectedFaceGroup);
    }
    if (m_setFaceGroupLocalMeshSizeAction) {
        m_setFaceGroupLocalMeshSizeAction->setEnabled(hasProject && hasSelectedFaceGroup);
    }
    if (m_toggleFaceGroupPhysicalGroupAction) {
        m_toggleFaceGroupPhysicalGroupAction->setEnabled(hasProject && hasSelectedFaceGroup);
    }
    if (m_createMaterialAction) {
        m_createMaterialAction->setEnabled(hasProject);
    }
    if (m_createBoundaryConditionAction) {
        m_createBoundaryConditionAction->setEnabled(hasProject);
    }
    if (m_createLoadAction) {
        m_createLoadAction->setEnabled(hasProject);
    }
    if (m_editSolverDataAction) {
        m_editSolverDataAction->setEnabled(hasProject && capabilities.canEditSolverData);
    }
    if (m_deleteSolverDataAction) {
        m_deleteSolverDataAction->setEnabled(hasProject && capabilities.canDeleteSolverData);
    }
    for (QAction *runSolverAction : m_runSolverActions) {
        if (runSolverAction) {
            runSolverAction->setEnabled(hasProject && runSolverAction->property("solverUsable").toBool());
        }
    }
}

WorkflowCommandContext MainWindow::workflowCommandContext()
{
    return WorkflowCommandContext{
        m_projectManager,
        m_geometryManager,
        m_projectModel,
        m_solverPluginManager,
        m_projectTreePanel,
        m_propertyPanel,
        m_renderView,
        m_logPanel,
        this,
        &m_pickController
    };
}

void MainWindow::writeLog(const QString &message)
{
    if (m_logPanel) {
        m_logPanel->appendMessage(message);
    }
}

void MainWindow::writeLogMessages(const QStringList &messages)
{
    for (const QString &message : messages) {
        writeLog(message);
    }
}
