#include "ui/MainWindowActions.h"

#include "commands/FaceGroupEditCommands.h"
#include "commands/GeometryCommands.h"
#include "commands/MeshCommands.h"
#include "commands/PickModeCommands.h"
#include "commands/ProjectCommands.h"
#include "commands/SolverCommands.h"
#include "commands/UndoStackController.h"
#include "commands/WorkflowCommandContext.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "ui/ActionRegistry.h"

#include <QAction>
#include <QActionGroup>
#include <QList>
#include <QMainWindow>
#include <QStringList>

namespace
{
constexpr auto CommandNewProject = "project.new";
constexpr auto CommandOpenProject = "project.open";
constexpr auto CommandExit = "app.exit";
constexpr auto CommandCreateBox = "geometry.create.box";
constexpr auto CommandCreateCylinder = "geometry.create.cylinder";
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
}

MainWindowActions MainWindowActionBuilder::build(
    QMainWindow *window,
    ActionRegistry &actionRegistry,
    const WorkflowCommandContext &context,
    UndoStackController &undoStackController,
    const MainWindowActionCallbacks &callbacks
)
{
    MainWindowActions actions;

    actions.newProject = new QAction("New Project", window);
    actions.newProject->setStatusTip("Create a new CAE project");
    actionRegistry.registerActionCommand(
        CommandNewProject,
        actions.newProject,
        window,
        makeProjectCommand(context, ProjectCommandType::Create)
    );

    actions.openProject = new QAction("Open Project", window);
    actions.openProject->setStatusTip("Open an existing CAE project");
    actionRegistry.registerActionCommand(
        CommandOpenProject,
        actions.openProject,
        window,
        makeProjectCommand(context, ProjectCommandType::Open)
    );

    actions.undo = undoStackController.stack().createUndoAction(window, "Undo");
    actions.redo = undoStackController.stack().createRedoAction(window, "Redo");

    for (int i = 0; i < 8; ++i) {
        QAction *recentAction = new QAction(window);
        recentAction->setVisible(false);
        actions.recentProjects.append(recentAction);
        QObject::connect(recentAction, &QAction::triggered, window, [recentAction, callbacks]() {
            if (callbacks.openRecentProject) {
                callbacks.openRecentProject(recentAction->data().toString());
            }
        });
    }
    actions.clearRecentProjects = new QAction("Clear Recent Projects", window);
    QObject::connect(actions.clearRecentProjects, &QAction::triggered, window, [callbacks]() {
        if (callbacks.clearRecentProjects) {
            callbacks.clearRecentProjects();
        }
    });

    actions.createBox = new QAction("Create Box", window);
    actions.createBox->setStatusTip("Create a box geometry from length, width, and height");
    actionRegistry.registerActionCommand(
        CommandCreateBox,
        actions.createBox,
        window,
        makeGeometryCreateCommand(context, GeometryCreateType::Box)
    );

    actions.createCylinder = new QAction("Create Cylinder", window);
    actions.createCylinder->setStatusTip("Create a cylinder geometry from radius and height");
    actionRegistry.registerActionCommand(
        CommandCreateCylinder,
        actions.createCylinder,
        window,
        makeGeometryCreateCommand(context, GeometryCreateType::Cylinder)
    );

    actions.checkGmsh = new QAction("Check Gmsh", window);
    actions.checkGmsh->setStatusTip("Run gmsh.exe --version");
    actionRegistry.registerActionCommand(
        CommandCheckGmsh,
        actions.checkGmsh,
        window,
        makeMeshCommand(context, MeshCommandType::CheckGmsh)
    );

    actions.generateMesh = new QAction("Generate Mesh", window);
    actions.generateMesh->setStatusTip("Generate a 3D mesh from the selected STEP geometry");
    actionRegistry.registerActionCommand(
        CommandGenerateMesh,
        actions.generateMesh,
        window,
        makeMeshCommand(context, MeshCommandType::Generate)
    );

    actions.readMeshInfo = new QAction("Read Mesh Info", window);
    actions.readMeshInfo->setStatusTip("Read node and tetrahedron counts from the selected MSH file");
    actionRegistry.registerActionCommand(
        CommandReadMeshInfo,
        actions.readMeshInfo,
        window,
        makeMeshCommand(context, MeshCommandType::ReadInfo)
    );

    actions.showMesh = new QAction("Show Mesh", window);
    actions.showMesh->setStatusTip("Read and display the selected tetrahedral MSH file");
    actionRegistry.registerActionCommand(
        CommandShowMesh,
        actions.showMesh,
        window,
        makeMeshCommand(context, MeshCommandType::Show)
    );

    actions.pickFace = new QAction("Pick Face", window);
    actions.pickFace->setCheckable(true);
    actions.pickFace->setStatusTip("Pick geometry faces in the render view");
    actionRegistry.registerActionCommand(
        CommandPickFace,
        actions.pickFace,
        window,
        makePickModeCommand(context)
    );

    actions.clearPick = new QAction("Clear Pick", window);
    actions.clearPick->setStatusTip("Clear the current pick highlight");
    actionRegistry.registerActionCommand(
        CommandClearPick,
        actions.clearPick,
        window,
        makeClearPickCommand(context)
    );

    actions.createFaceGroupFromPick = new QAction("Create Face Group from Pick", window);
    actions.createFaceGroupFromPick->setStatusTip("Create or update a face group from the picked face");
    actionRegistry.registerActionCommand(
        CommandCreateFaceGroupFromPick,
        actions.createFaceGroupFromPick,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::CreateFromPick)
    );

    actions.addPickedFacesToFaceGroup = new QAction("Add Picked Faces to Face Group", window);
    actionRegistry.registerActionCommand(
        CommandAddPickedFacesToFaceGroup,
        actions.addPickedFacesToFaceGroup,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::AddPickedFaces)
    );

    actions.removePickedFacesFromFaceGroup = new QAction("Remove Picked Faces from Face Group", window);
    actionRegistry.registerActionCommand(
        CommandRemovePickedFacesFromFaceGroup,
        actions.removePickedFacesFromFaceGroup,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::RemovePickedFaces)
    );

    actions.clearFaceGroupFaces = new QAction("Clear Selected Face Group Faces", window);
    actionRegistry.registerActionCommand(
        CommandClearFaceGroupFaces,
        actions.clearFaceGroupFaces,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::ClearFaces)
    );

    actions.renameFaceGroup = new QAction("Rename Selected Face Group", window);
    actionRegistry.registerActionCommand(
        CommandRenameFaceGroup,
        actions.renameFaceGroup,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::Rename)
    );

    actions.deleteFaceGroup = new QAction("Delete Selected Face Group", window);
    actionRegistry.registerActionCommand(
        CommandDeleteFaceGroup,
        actions.deleteFaceGroup,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::Delete)
    );

    actions.setFaceGroupLocalMeshSize = new QAction("Set Face Group Local Mesh Size", window);
    actionRegistry.registerActionCommand(
        CommandSetFaceGroupLocalMeshSize,
        actions.setFaceGroupLocalMeshSize,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::SetLocalMeshSize)
    );

    actions.toggleFaceGroupPhysicalGroup = new QAction("Toggle Face Group Physical Group", window);
    actionRegistry.registerActionCommand(
        CommandToggleFaceGroupPhysicalGroup,
        actions.toggleFaceGroupPhysicalGroup,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::TogglePhysicalGroup)
    );

    actions.exit = new QAction("Exit", window);
    actionRegistry.registerActionCommand(CommandExit, actions.exit, window, makeCloseWindowCommand(window));

    actions.createMaterial = new QAction("Create Material", window);
    actions.createMaterial->setStatusTip("Create a new material");
    actionRegistry.registerActionCommand(
        CommandCreateMaterial,
        actions.createMaterial,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateMaterial)
    );

    actions.createBoundaryCondition = new QAction("Create Boundary Condition", window);
    actions.createBoundaryCondition->setStatusTip("Create a new boundary condition");
    actionRegistry.registerActionCommand(
        CommandCreateBoundaryCondition,
        actions.createBoundaryCondition,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateBoundaryCondition)
    );

    actions.createLoad = new QAction("Create Load", window);
    actions.createLoad->setStatusTip("Create a new load");
    actionRegistry.registerActionCommand(
        CommandCreateLoad,
        actions.createLoad,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateLoad)
    );

    actions.editSolverData = new QAction("Edit Selected Solver Data", window);
    actions.editSolverData->setStatusTip("Edit the selected material, boundary condition, or load");
    actionRegistry.registerActionCommand(
        CommandEditSolverData,
        actions.editSolverData,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::EditSelected)
    );

    actions.deleteSolverData = new QAction("Delete Selected Solver Data", window);
    actions.deleteSolverData->setStatusTip("Delete the selected material, boundary condition, or load");
    actionRegistry.registerActionCommand(
        CommandDeleteSolverData,
        actions.deleteSolverData,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::DeleteSelected)
    );

    const QStringList resultFields{
        CalculiXResultFields::Ux,
        CalculiXResultFields::Uy,
        CalculiXResultFields::Uz,
        CalculiXResultFields::DisplacementMagnitude,
        CalculiXResultFields::VonMisesStress
    };
    auto *fieldGroup = new QActionGroup(window);
    fieldGroup->setExclusive(true);
    for (const QString &field : resultFields) {
        QAction *fieldAction = new QAction(field, window);
        fieldAction->setCheckable(true);
        fieldAction->setData(field);
        fieldGroup->addAction(fieldAction);
        actions.resultFields.append(fieldAction);
        QObject::connect(fieldAction, &QAction::triggered, window, [field, callbacks]() {
            if (callbacks.setSelectedResultField) {
                callbacks.setSelectedResultField(field);
            }
        });
    }

    const QList<double> resultScales{0.0, 1.0, 10.0, 100.0};
    auto *scaleGroup = new QActionGroup(window);
    scaleGroup->setExclusive(true);
    for (double scale : resultScales) {
        QAction *scaleAction = new QAction(QString("Deformation %1x").arg(scale, 0, 'g', 6), window);
        scaleAction->setCheckable(true);
        scaleAction->setData(scale);
        scaleGroup->addAction(scaleAction);
        actions.resultScales.append(scaleAction);
        QObject::connect(scaleAction, &QAction::triggered, window, [scale, callbacks]() {
            if (callbacks.setSelectedResultDeformationScale) {
                callbacks.setSelectedResultDeformationScale(scale);
            }
        });
    }

    actions.exportScreenshot = new QAction("Export Render Screenshot", window);
    QObject::connect(actions.exportScreenshot, &QAction::triggered, window, [callbacks]() {
        if (callbacks.exportRenderScreenshot) {
            callbacks.exportRenderScreenshot();
        }
    });

    actions.projectResources = new QAction("Project Resources", window);
    QObject::connect(actions.projectResources, &QAction::triggered, window, [callbacks]() {
        if (callbacks.showProjectResources) {
            callbacks.showProjectResources();
        }
    });

    actions.validateSamples = new QAction("Validate Samples", window);
    QObject::connect(actions.validateSamples, &QAction::triggered, window, [callbacks]() {
        if (callbacks.validateSamples) {
            callbacks.validateSamples();
        }
    });

    actions.clearDiagnostics = new QAction("Clear Diagnostics", window);
    QObject::connect(actions.clearDiagnostics, &QAction::triggered, window, [callbacks]() {
        if (callbacks.clearDiagnostics) {
            callbacks.clearDiagnostics();
        }
    });

    return actions;
}
