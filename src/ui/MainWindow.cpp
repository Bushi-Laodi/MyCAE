#include "MainWindow.h"

#include "DiagnosticPanel.h"
#include "LogPanel.h"
#include "ProjectTreePanel.h"
#include "PropertyPanel.h"
#include "ProjectResourceDialog.h"
#include "RenderView.h"
#include "ResultPostprocessPanel.h"
#include "SampleValidationDialog.h"
#include "commands/FaceGroupEditCommands.h"
#include "commands/GeometryCommands.h"
#include "commands/MeshCommands.h"
#include "commands/PickModeCommands.h"
#include "commands/ProjectCommands.h"
#include "commands/SolverCommands.h"
#include "commands/UtilityCommands.h"
#include "commands/WorkflowCommandContext.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "solver/plugin/SolverPluginDescriptorFormatter.h"
#include "workflow/ProjectWorkflowController.h"
#include "workflow/ResultWorkflowController.h"
#include "workflow/SelectionController.h"

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QDockWidget>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QShowEvent>
#include <QStatusBar>
#include <QToolBar>
#include <QTimer>

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
    m_undoStackController.setFaceGroupRestoreCallback([this](const QString &selectionId) {
        handleUndoStackFaceGroupsChanged(selectionId);
    });
    m_actionRegistry.setAfterExecuteCallback([this]() {
        if (m_projectModel.hasProject()) {
            if (m_activeProjectFile != m_projectModel.project().projectFilePath) {
                m_undoStackController.clear();
                m_activeProjectFile = m_projectModel.project().projectFilePath;
            }
            m_appSettings.addRecentProject(m_projectModel.project().projectFilePath);
            updateRecentProjectActions();
        }
        if (m_resultPostprocessPanel) {
            m_resultPostprocessPanel->setResult(m_projectModel.resultForSelection());
        }
        updateActionStates();
    });
    connect(
        &m_resultAnimationController,
        &ResultAnimationController::frameScaleChanged,
        this,
        &MainWindow::applyAnimatedResultDeformationScale
    );
    updateActionStates();
    updateRecentProjectActions();
    m_appSettings.restoreMainWindow(*this);

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

    m_undoAction = m_undoStackController.stack().createUndoAction(this, "Undo");
    m_redoAction = m_undoStackController.stack().createRedoAction(this, "Redo");

    for (int i = 0; i < 8; ++i) {
        QAction *recentAction = new QAction(this);
        recentAction->setVisible(false);
        m_recentProjectActions.append(recentAction);
        connect(recentAction, &QAction::triggered, this, [this, recentAction]() {
            openRecentProject(recentAction->data().toString());
        });
    }
    m_clearRecentProjectsAction = new QAction("Clear Recent Projects", this);
    connect(m_clearRecentProjectsAction, &QAction::triggered, this, &MainWindow::clearRecentProjects);

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

    const QStringList resultFields{
        CalculiXResultFields::Ux,
        CalculiXResultFields::Uy,
        CalculiXResultFields::Uz,
        CalculiXResultFields::DisplacementMagnitude,
        CalculiXResultFields::VonMisesStress
    };
    auto *fieldGroup = new QActionGroup(this);
    fieldGroup->setExclusive(true);
    for (const QString &field : resultFields) {
        QAction *fieldAction = new QAction(field, this);
        fieldAction->setCheckable(true);
        fieldAction->setData(field);
        fieldGroup->addAction(fieldAction);
        m_resultFieldActions.append(fieldAction);
        connect(fieldAction, &QAction::triggered, this, [this, field]() {
            setSelectedResultField(field);
        });
    }

    const QList<double> resultScales{0.0, 1.0, 10.0, 100.0};
    auto *scaleGroup = new QActionGroup(this);
    scaleGroup->setExclusive(true);
    for (double scale : resultScales) {
        QAction *scaleAction = new QAction(QString("Deformation %1x").arg(scale, 0, 'g', 6), this);
        scaleAction->setCheckable(true);
        scaleAction->setData(scale);
        scaleGroup->addAction(scaleAction);
        m_resultScaleActions.append(scaleAction);
        connect(scaleAction, &QAction::triggered, this, [this, scale]() {
            setSelectedResultDeformationScale(scale);
        });
    }

    m_exportScreenshotAction = new QAction("Export Render Screenshot", this);
    connect(m_exportScreenshotAction, &QAction::triggered, this, &MainWindow::exportRenderScreenshot);

    m_projectResourcesAction = new QAction("Project Resources", this);
    connect(m_projectResourcesAction, &QAction::triggered, this, &MainWindow::showProjectResources);

    m_validateSamplesAction = new QAction("Validate Samples", this);
    connect(m_validateSamplesAction, &QAction::triggered, this, &MainWindow::validateSamples);

    m_clearDiagnosticsAction = new QAction("Clear Diagnostics", this);
    connect(m_clearDiagnosticsAction, &QAction::triggered, this, &MainWindow::clearDiagnostics);
}

void MainWindow::createMenus()
{
    const WorkflowCommandContext context = workflowCommandContext();

    auto *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(m_newProjectAction);
    fileMenu->addAction(m_openProjectAction);
    m_recentProjectsMenu = fileMenu->addMenu("Recent Projects");
    for (QAction *recentAction : m_recentProjectActions) {
        m_recentProjectsMenu->addAction(recentAction);
    }
    m_recentProjectsMenu->addSeparator();
    m_recentProjectsMenu->addAction(m_clearRecentProjectsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    auto *editMenu = menuBar()->addMenu("Edit");
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);

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

    auto *postMenu = menuBar()->addMenu("Postprocess");
    auto *fieldMenu = postMenu->addMenu("Result Field");
    for (QAction *fieldAction : m_resultFieldActions) {
        fieldMenu->addAction(fieldAction);
    }
    auto *scaleMenu = postMenu->addMenu("Deformation Scale");
    for (QAction *scaleAction : m_resultScaleActions) {
        scaleMenu->addAction(scaleAction);
    }
    postMenu->addSeparator();
    postMenu->addAction(m_exportScreenshotAction);

    auto *toolsMenu = menuBar()->addMenu("Tools");
    toolsMenu->addAction(m_projectResourcesAction);
    toolsMenu->addAction(m_validateSamplesAction);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_clearDiagnosticsAction);
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

    auto *diagnosticDock = new QDockWidget("Diagnostics", this);
    m_diagnosticPanel = new DiagnosticPanel(diagnosticDock);
    diagnosticDock->setWidget(m_diagnosticPanel);
    addDockWidget(Qt::BottomDockWidgetArea, diagnosticDock);

    auto *propertyDock = new QDockWidget("Properties", this);
    m_propertyPanel = new PropertyPanel(propertyDock);
    propertyDock->setWidget(m_propertyPanel);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock);

    auto *postprocessDock = new QDockWidget("Result Postprocess", this);
    m_resultPostprocessPanel = new ResultPostprocessPanel(postprocessDock);
    connect(m_resultPostprocessPanel, &ResultPostprocessPanel::fieldChanged, this, &MainWindow::setSelectedResultField);
    connect(
        m_resultPostprocessPanel,
        &ResultPostprocessPanel::deformationScaleChanged,
        this,
        &MainWindow::setSelectedResultDeformationScale
    );
    connect(m_resultPostprocessPanel, &ResultPostprocessPanel::meshEdgesChanged, this, &MainWindow::setSelectedResultMeshEdges);
    connect(
        m_resultPostprocessPanel,
        &ResultPostprocessPanel::undeformedOverlayChanged,
        this,
        &MainWindow::setSelectedResultUndeformedOverlay
    );
    connect(
        m_resultPostprocessPanel,
        &ResultPostprocessPanel::animationPlayRequested,
        this,
        &MainWindow::playSelectedResultAnimation
    );
    connect(
        m_resultPostprocessPanel,
        &ResultPostprocessPanel::animationStopRequested,
        this,
        &MainWindow::stopSelectedResultAnimation
    );
    connect(
        m_resultPostprocessPanel,
        &ResultPostprocessPanel::exportCsvRequested,
        this,
        &MainWindow::exportSelectedResultCsv
    );
    connect(
        m_resultPostprocessPanel,
        &ResultPostprocessPanel::exportReportRequested,
        this,
        &MainWindow::exportSelectedResultReport
    );
    connect(
        m_resultPostprocessPanel,
        &ResultPostprocessPanel::exportScreenshotRequested,
        this,
        &MainWindow::exportRenderScreenshot
    );
    connect(
        m_resultPostprocessPanel,
        &ResultPostprocessPanel::openResultDirectoryRequested,
        this,
        &MainWindow::openSelectedResultDirectory
    );
    connect(m_resultPostprocessPanel, &ResultPostprocessPanel::renameResultRequested, this, &MainWindow::renameSelectedResult);
    connect(
        m_resultPostprocessPanel,
        &ResultPostprocessPanel::deleteResultRequested,
        this,
        &MainWindow::deleteSelectedResultHistory
    );
    postprocessDock->setWidget(m_resultPostprocessPanel);
    addDockWidget(Qt::RightDockWidgetArea, postprocessDock);
    tabifyDockWidget(propertyDock, postprocessDock);

    auto *logDock = new QDockWidget("Log", this);
    m_logPanel = new LogPanel(logDock);
    logDock->setWidget(m_logPanel);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
    tabifyDockWidget(logDock, diagnosticDock);
}

void MainWindow::applySelection(const Selection &selection)
{
    const ResultObject *previousResult = m_projectModel.resultForSelection();
    if (m_resultAnimationController.isRunning()
            && (selection.kind != SelectionKind::Result
                || !previousResult
                || selection.id != previousResult->id)) {
        stopSelectedResultAnimation();
    }

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
    if (m_resultPostprocessPanel) {
        m_resultPostprocessPanel->setResult(m_projectModel.resultForSelection());
    }
    if (selection.kind != SelectionKind::FaceGroup) {
        m_pickController.clear(m_renderView);
    }
    updateActionStates();
}

void MainWindow::setSelectedResultField(const QString &fieldName)
{
    writeLogMessages(resultWorkflowController().setSelectedField(fieldName));
    updateActionStates();
}

void MainWindow::setSelectedResultDeformationScale(double scale)
{
    writeLogMessages(resultWorkflowController().setSelectedDeformationScale(scale));
    updateActionStates();
}

void MainWindow::setSelectedResultMeshEdges(bool enabled)
{
    writeLogMessages(resultWorkflowController().setSelectedMeshEdges(enabled));
    updateActionStates();
}

void MainWindow::setSelectedResultUndeformedOverlay(bool enabled)
{
    writeLogMessages(resultWorkflowController().setSelectedUndeformedOverlay(enabled));
    updateActionStates();
}

void MainWindow::playSelectedResultAnimation(double speed)
{
    writeLogMessages(resultWorkflowController().playSelectedAnimation(speed));
    updateActionStates();
}

void MainWindow::stopSelectedResultAnimation()
{
    writeLogMessages(resultWorkflowController().stopSelectedAnimation());
    updateActionStates();
}

void MainWindow::applyAnimatedResultDeformationScale(double scale)
{
    writeLogMessages(resultWorkflowController().applyAnimatedDeformationScale(scale));
}

void MainWindow::showProjectResources()
{
    if (!m_projectModel.hasProject()) {
        writeLog("Project resources skipped: open a project first.");
        return;
    }

    ProjectResourceDialog dialog(m_projectModel, this);
    connect(&dialog, &ProjectResourceDialog::logMessagesReady, this, &MainWindow::writeLogMessages);
    connect(&dialog, &ProjectResourceDialog::resultsChanged, this, &MainWindow::refreshResultViews);
    dialog.exec();
}

void MainWindow::openRecentProject(const QString &projectFilePath)
{
    if (projectFilePath.trimmed().isEmpty()) {
        return;
    }

    ProjectWorkflowController projectWorkflow(
        m_projectManager,
        m_geometryManager,
        m_projectModel,
        m_projectTreePanel,
        m_propertyPanel,
        m_renderView,
        this
    );
    const ProjectWorkflowResult result = projectWorkflow.openProjectFile(projectFilePath);
    writeLogMessages(result.logMessages);
    if (result.success) {
        m_activeProjectFile = m_projectModel.project().projectFilePath;
        m_undoStackController.clear();
        m_appSettings.addRecentProject(m_activeProjectFile);
        updateRecentProjectActions();
    }
    updateActionStates();
}

void MainWindow::updateRecentProjectActions()
{
    const QStringList projects = m_appSettings.recentProjects();
    for (int i = 0; i < m_recentProjectActions.size(); ++i) {
        QAction *action = m_recentProjectActions.at(i);
        if (!action) {
            continue;
        }
        const bool visible = i < projects.size();
        action->setVisible(visible);
        if (!visible) {
            continue;
        }
        const QString path = projects.at(i);
        action->setText(QFileInfo(path).dir().dirName() + " - " + QDir::toNativeSeparators(path));
        action->setData(path);
    }
    if (m_recentProjectsMenu) {
        m_recentProjectsMenu->setEnabled(!projects.isEmpty());
    }
    if (m_clearRecentProjectsAction) {
        m_clearRecentProjectsAction->setEnabled(!projects.isEmpty());
    }
}

void MainWindow::clearRecentProjects()
{
    m_appSettings.clearRecentProjects();
    updateRecentProjectActions();
    writeLog("Recent projects cleared.");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_appSettings.saveMainWindow(*this);
    QMainWindow::closeEvent(event);
}

void MainWindow::clearDiagnostics()
{
    m_diagnosticCollector.clear();
    refreshDiagnosticsPanel();
    writeLog("Diagnostics cleared.");
}

void MainWindow::validateSamples()
{
    SampleValidationDialog dialog(this);
    connect(&dialog, &SampleValidationDialog::logMessagesReady, this, &MainWindow::writeLogMessages);
    QTimer::singleShot(0, &dialog, &SampleValidationDialog::runValidation);
    dialog.exec();
}

void MainWindow::refreshDiagnosticsPanel()
{
    if (m_diagnosticPanel) {
        m_diagnosticPanel->setDiagnostics(m_diagnosticCollector.diagnostics());
    }
}

void MainWindow::refreshResultViews()
{
    if (m_projectTreePanel) {
        m_projectTreePanel->setResultItems(m_projectModel.resultRepository().results());
    }
    if (m_propertyPanel) {
        m_propertyPanel->showResultCategory(m_projectModel.resultRepository().results());
    }
    if (m_resultPostprocessPanel) {
        m_resultPostprocessPanel->setResult(m_projectModel.resultForSelection());
    }
    updateActionStates();
}

void MainWindow::handleUndoStackFaceGroupsChanged(const QString &selectionId)
{
    ProjectWorkflowController projectWorkflow(
        m_projectManager,
        m_geometryManager,
        m_projectModel,
        m_projectTreePanel,
        m_propertyPanel,
        m_renderView,
        this
    );
    projectWorkflow.refreshFaceGroupTree();
    writeLogMessages(projectWorkflow.saveSimulationCase().logMessages);

    if (!selectionId.isEmpty() && m_projectModel.findFaceGroupById(selectionId)) {
        applySelection(Selection::item(SelectionKind::FaceGroup, selectionId));
    } else {
        m_projectModel.clearSelectionIfKind(SelectionKind::FaceGroup);
        if (m_propertyPanel) {
            m_propertyPanel->showEmptySelection();
        }
    }
    updateActionStates();
}

void MainWindow::exportSelectedResultCsv()
{
    writeLogMessages(resultWorkflowController().exportSelectedCsv());
}

void MainWindow::exportSelectedResultReport()
{
    writeLogMessages(resultWorkflowController().exportSelectedReport());
}

void MainWindow::exportRenderScreenshot()
{
    writeLogMessages(resultWorkflowController().exportRenderScreenshot());
}

void MainWindow::openSelectedResultDirectory()
{
    writeLogMessages(resultWorkflowController().openSelectedResultDirectory());
}

void MainWindow::renameSelectedResult()
{
    writeLogMessages(resultWorkflowController().renameSelectedResult());
    updateActionStates();
}

void MainWindow::deleteSelectedResultHistory()
{
    writeLogMessages(resultWorkflowController().deleteSelectedResultHistory());
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
    const ResultObject *selectedResult = m_projectModel.resultForSelection();

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
    for (QAction *fieldAction : m_resultFieldActions) {
        if (!fieldAction) {
            continue;
        }
        fieldAction->setEnabled(hasProject && selectedResult);
        if (selectedResult) {
            const QString field = fieldAction->data().toString();
            fieldAction->setChecked(field == selectedResult->displayFieldName
                || (selectedResult->displayFieldName.isEmpty() && field == selectedResult->primaryFieldName));
        }
    }
    for (QAction *scaleAction : m_resultScaleActions) {
        if (!scaleAction) {
            continue;
        }
        scaleAction->setEnabled(hasProject && selectedResult);
        if (selectedResult) {
            scaleAction->setChecked(scaleAction->data().toDouble() == selectedResult->deformationScale);
        }
    }
    if (m_exportScreenshotAction) {
        m_exportScreenshotAction->setEnabled(hasProject);
    }
    if (m_projectResourcesAction) {
        m_projectResourcesAction->setEnabled(hasProject);
    }
    if (m_clearDiagnosticsAction) {
        m_clearDiagnosticsAction->setEnabled(!m_diagnosticCollector.diagnostics().isEmpty());
    }
    if (m_resultPostprocessPanel) {
        m_resultPostprocessPanel->setEnabledForResult(hasProject && selectedResult);
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
        &m_pickController,
        &m_undoStackController
    };
}

ResultWorkflowController MainWindow::resultWorkflowController()
{
    return ResultWorkflowController(
        m_projectModel,
        m_projectTreePanel,
        m_propertyPanel,
        m_resultPostprocessPanel,
        m_renderView,
        m_appSettings,
        m_resultAnimationController,
        this
    );
}

void MainWindow::writeLog(const QString &message)
{
    if (m_logPanel) {
        m_logPanel->appendMessage(message);
    }
    if (m_diagnosticCollector.addFromLogMessage(message)) {
        refreshDiagnosticsPanel();
    }
}

void MainWindow::writeLogMessages(const QStringList &messages)
{
    for (const QString &message : messages) {
        writeLog(message);
    }
}
