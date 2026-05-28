#include "ui/MainWindowActions.h"

#include "commands/DisplayModeCommands.h"
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
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

constexpr auto CommandNewProject = "project.new";
constexpr auto CommandOpenProject = "project.open";
constexpr auto CommandExit = "app.exit";
constexpr auto CommandCreateBox = "geometry.create.box";
constexpr auto CommandCreateCylinder = "geometry.create.cylinder";
constexpr auto CommandCreateSphere = "geometry.create.sphere";
constexpr auto CommandImportStep = "geometry.import.step";
constexpr auto CommandCreateBoolean = "geometry.create.boolean";
constexpr auto CommandDeleteGeometry = "geometry.deleteSelected";
constexpr auto CommandToggleGeometryEdges = "display.geometryEdges.toggle";
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
constexpr auto CommandCreateStructuralMaterial = "solverData.create.structuralMaterial";
constexpr auto CommandCreateFluidMaterial = "solverData.create.fluidMaterial";
constexpr auto CommandCreateBoundaryCondition = "solverData.create.boundaryCondition";
constexpr auto CommandCreateStructuralBoundaryCondition = "solverData.create.structuralBoundaryCondition";
constexpr auto CommandCreateCfdBoundaryCondition = "solverData.create.cfdBoundaryCondition";
constexpr auto CommandCreateLoad = "solverData.create.load";
constexpr auto CommandCreateStructuralLoad = "solverData.create.structuralLoad";
constexpr auto CommandCreateCfdFieldValue = "solverData.create.cfdFieldValue";
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

    actions.newProject = new QAction(zh(u8"新建工程"), window);
    actions.newProject->setStatusTip(zh(u8"创建新的 CAE 工程"));
    actionRegistry.registerActionCommand(
        CommandNewProject,
        actions.newProject,
        window,
        makeProjectCommand(context, ProjectCommandType::Create)
    );

    actions.openProject = new QAction(zh(u8"打开工程"), window);
    actions.openProject->setStatusTip(zh(u8"打开已有 CAE 工程"));
    actionRegistry.registerActionCommand(
        CommandOpenProject,
        actions.openProject,
        window,
        makeProjectCommand(context, ProjectCommandType::Open)
    );

    actions.undo = undoStackController.stack().createUndoAction(window, zh(u8"撤销"));
    actions.redo = undoStackController.stack().createRedoAction(window, zh(u8"重做"));

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
    actions.clearRecentProjects = new QAction(zh(u8"清空最近工程"), window);
    QObject::connect(actions.clearRecentProjects, &QAction::triggered, window, [callbacks]() {
        if (callbacks.clearRecentProjects) {
            callbacks.clearRecentProjects();
        }
    });

    actions.createBox = new QAction(zh(u8"创建长方体"), window);
    actions.createBox->setStatusTip(zh(u8"通过长、宽、高创建长方体几何"));
    actionRegistry.registerActionCommand(
        CommandCreateBox,
        actions.createBox,
        window,
        makeGeometryCreateCommand(context, GeometryCreateType::Box)
    );

    actions.createCylinder = new QAction(zh(u8"创建圆柱体"), window);
    actions.createCylinder->setStatusTip(zh(u8"通过半径和高度创建圆柱体几何"));
    actionRegistry.registerActionCommand(
        CommandCreateCylinder,
        actions.createCylinder,
        window,
        makeGeometryCreateCommand(context, GeometryCreateType::Cylinder)
    );

    actions.createSphere = new QAction(zh(u8"创建球体"), window);
    actions.createSphere->setStatusTip(zh(u8"通过中心点和半径创建球体几何"));
    actionRegistry.registerActionCommand(
        CommandCreateSphere,
        actions.createSphere,
        window,
        makeGeometryCreateCommand(context, GeometryCreateType::Sphere)
    );

    actions.importStep = new QAction(zh(u8"导入 STEP"), window);
    actions.importStep->setStatusTip(zh(u8"导入 STEP/STP 文件为可显示、可网格、可布尔的几何对象"));
    actionRegistry.registerActionCommand(
        CommandImportStep,
        actions.importStep,
        window,
        makeGeometryImportStepCommand(context)
    );

    actions.createBoolean = new QAction(zh(u8"布尔操作"), window);
    actions.createBoolean->setStatusTip(zh(u8"对两个几何体执行并集、切除或交集操作"));
    actionRegistry.registerActionCommand(
        CommandCreateBoolean,
        actions.createBoolean,
        window,
        makeGeometryBooleanCommand(context)
    );

    actions.deleteGeometry = new QAction(zh(u8"删除选中几何体"), window);
    actions.deleteGeometry->setStatusTip(zh(u8"删除当前选中的几何体，并清理关联的网格、面组和求解引用"));
    actionRegistry.registerActionCommand(
        CommandDeleteGeometry,
        actions.deleteGeometry,
        window,
        makeGeometryDeleteSelectedCommand(context)
    );

    actions.showGeometryEdges = new QAction(zh(u8"显示几何边线"), window);
    actions.showGeometryEdges->setCheckable(true);
    actions.showGeometryEdges->setStatusTip(zh(u8"普通查看时显示或隐藏几何三角剖分边线；拾取面模式会临时自动显示边线"));
    actionRegistry.registerActionCommand(
        CommandToggleGeometryEdges,
        actions.showGeometryEdges,
        window,
        makeToggleGeometryEdgesCommand(context)
    );

    actions.checkGmsh = new QAction(zh(u8"检查 Gmsh"), window);
    actions.checkGmsh->setStatusTip("Run gmsh.exe --version");
    actionRegistry.registerActionCommand(
        CommandCheckGmsh,
        actions.checkGmsh,
        window,
        makeMeshCommand(context, MeshCommandType::CheckGmsh)
    );

    actions.generateMesh = new QAction(zh(u8"生成网格"), window);
    actions.generateMesh->setStatusTip(zh(u8"基于选中的 STEP 几何生成三维网格"));
    actionRegistry.registerActionCommand(
        CommandGenerateMesh,
        actions.generateMesh,
        window,
        makeMeshCommand(context, MeshCommandType::Generate)
    );

    actions.readMeshInfo = new QAction(zh(u8"读取网格信息"), window);
    actions.readMeshInfo->setStatusTip(zh(u8"读取选中 MSH 文件的节点和四面体数量"));
    actionRegistry.registerActionCommand(
        CommandReadMeshInfo,
        actions.readMeshInfo,
        window,
        makeMeshCommand(context, MeshCommandType::ReadInfo)
    );

    actions.showMesh = new QAction(zh(u8"显示网格"), window);
    actions.showMesh->setStatusTip(zh(u8"读取并显示选中的四面体 MSH 文件"));
    actionRegistry.registerActionCommand(
        CommandShowMesh,
        actions.showMesh,
        window,
        makeMeshCommand(context, MeshCommandType::Show)
    );

    actions.pickFace = new QAction(zh(u8"拾取面"), window);
    actions.pickFace->setCheckable(true);
    actions.pickFace->setStatusTip(zh(u8"在渲染视图中拾取几何面"));
    actionRegistry.registerActionCommand(
        CommandPickFace,
        actions.pickFace,
        window,
        makePickModeCommand(context)
    );

    actions.clearPick = new QAction(zh(u8"清除拾取"), window);
    actions.clearPick->setStatusTip(zh(u8"清除当前拾取高亮"));
    actionRegistry.registerActionCommand(
        CommandClearPick,
        actions.clearPick,
        window,
        makeClearPickCommand(context)
    );

    actions.createFaceGroupFromPick = new QAction(zh(u8"从拾取创建面组"), window);
    actions.createFaceGroupFromPick->setStatusTip(zh(u8"基于拾取面创建或更新面组"));
    actionRegistry.registerActionCommand(
        CommandCreateFaceGroupFromPick,
        actions.createFaceGroupFromPick,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::CreateFromPick)
    );

    actions.addPickedFacesToFaceGroup = new QAction(zh(u8"将拾取面加入面组"), window);
    actionRegistry.registerActionCommand(
        CommandAddPickedFacesToFaceGroup,
        actions.addPickedFacesToFaceGroup,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::AddPickedFaces)
    );

    actions.removePickedFacesFromFaceGroup = new QAction(zh(u8"从面组移除拾取面"), window);
    actionRegistry.registerActionCommand(
        CommandRemovePickedFacesFromFaceGroup,
        actions.removePickedFacesFromFaceGroup,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::RemovePickedFaces)
    );

    actions.clearFaceGroupFaces = new QAction(zh(u8"清空选中面组"), window);
    actionRegistry.registerActionCommand(
        CommandClearFaceGroupFaces,
        actions.clearFaceGroupFaces,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::ClearFaces)
    );

    actions.renameFaceGroup = new QAction(zh(u8"重命名选中面组"), window);
    actionRegistry.registerActionCommand(
        CommandRenameFaceGroup,
        actions.renameFaceGroup,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::Rename)
    );

    actions.deleteFaceGroup = new QAction(zh(u8"删除选中面组"), window);
    actionRegistry.registerActionCommand(
        CommandDeleteFaceGroup,
        actions.deleteFaceGroup,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::Delete)
    );

    actions.setFaceGroupLocalMeshSize = new QAction(zh(u8"设置面组局部网格尺寸"), window);
    actionRegistry.registerActionCommand(
        CommandSetFaceGroupLocalMeshSize,
        actions.setFaceGroupLocalMeshSize,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::SetLocalMeshSize)
    );

    actions.toggleFaceGroupPhysicalGroup = new QAction(zh(u8"切换面组物理组"), window);
    actionRegistry.registerActionCommand(
        CommandToggleFaceGroupPhysicalGroup,
        actions.toggleFaceGroupPhysicalGroup,
        window,
        makeFaceGroupEditCommand(context, FaceGroupEditCommandType::TogglePhysicalGroup)
    );

    actions.exit = new QAction(zh(u8"退出"), window);
    actionRegistry.registerActionCommand(CommandExit, actions.exit, window, makeCloseWindowCommand(window));

    actions.createMaterial = new QAction(zh(u8"创建材料"), window);
    actions.createMaterial->setStatusTip(zh(u8"创建新材料"));
    actionRegistry.registerActionCommand(
        CommandCreateMaterial,
        actions.createMaterial,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateMaterial)
    );

    actions.createStructuralMaterial = new QAction(zh(u8"创建结构材料"), window);
    actionRegistry.registerActionCommand(
        CommandCreateStructuralMaterial,
        actions.createStructuralMaterial,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateStructuralMaterial)
    );

    actions.createFluidMaterial = new QAction(zh(u8"创建流体材料"), window);
    actionRegistry.registerActionCommand(
        CommandCreateFluidMaterial,
        actions.createFluidMaterial,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateFluidMaterial)
    );

    actions.createBoundaryCondition = new QAction(zh(u8"创建边界条件"), window);
    actions.createBoundaryCondition->setStatusTip(zh(u8"创建新的边界条件"));
    actionRegistry.registerActionCommand(
        CommandCreateBoundaryCondition,
        actions.createBoundaryCondition,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateBoundaryCondition)
    );

    actions.createStructuralBoundaryCondition = new QAction(zh(u8"创建结构约束"), window);
    actionRegistry.registerActionCommand(
        CommandCreateStructuralBoundaryCondition,
        actions.createStructuralBoundaryCondition,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateStructuralBoundaryCondition)
    );

    actions.createCfdBoundaryCondition = new QAction(zh(u8"创建 CFD 边界"), window);
    actionRegistry.registerActionCommand(
        CommandCreateCfdBoundaryCondition,
        actions.createCfdBoundaryCondition,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateCfdBoundaryCondition)
    );

    actions.createLoad = new QAction(zh(u8"创建载荷"), window);
    actions.createLoad->setStatusTip(zh(u8"创建新载荷"));
    actionRegistry.registerActionCommand(
        CommandCreateLoad,
        actions.createLoad,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateLoad)
    );

    actions.createStructuralLoad = new QAction(zh(u8"创建结构载荷"), window);
    actionRegistry.registerActionCommand(
        CommandCreateStructuralLoad,
        actions.createStructuralLoad,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateStructuralLoad)
    );

    actions.createCfdFieldValue = new QAction(zh(u8"创建 CFD 场值"), window);
    actionRegistry.registerActionCommand(
        CommandCreateCfdFieldValue,
        actions.createCfdFieldValue,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::CreateCfdFieldValue)
    );

    actions.editSolverData = new QAction(zh(u8"编辑选中求解数据"), window);
    actions.editSolverData->setStatusTip(zh(u8"编辑选中的材料、边界条件或载荷"));
    actionRegistry.registerActionCommand(
        CommandEditSolverData,
        actions.editSolverData,
        window,
        makeSolverDataCommand(context, SolverDataCommandType::EditSelected)
    );

    actions.deleteSolverData = new QAction(zh(u8"删除选中求解数据"), window);
    actions.deleteSolverData->setStatusTip(zh(u8"删除选中的材料、边界条件或载荷"));
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
        QAction *scaleAction = new QAction(zh(u8"变形比例 %1x").arg(scale, 0, 'g', 6), window);
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

    actions.exportScreenshot = new QAction(zh(u8"导出渲染截图"), window);
    QObject::connect(actions.exportScreenshot, &QAction::triggered, window, [callbacks]() {
        if (callbacks.exportRenderScreenshot) {
            callbacks.exportRenderScreenshot();
        }
    });

    actions.projectResources = new QAction(zh(u8"工程资源"), window);
    QObject::connect(actions.projectResources, &QAction::triggered, window, [callbacks]() {
        if (callbacks.showProjectResources) {
            callbacks.showProjectResources();
        }
    });

    actions.validateSamples = new QAction(zh(u8"验证样例"), window);
    QObject::connect(actions.validateSamples, &QAction::triggered, window, [callbacks]() {
        if (callbacks.validateSamples) {
            callbacks.validateSamples();
        }
    });

    actions.clearDiagnostics = new QAction(zh(u8"清空诊断"), window);
    QObject::connect(actions.clearDiagnostics, &QAction::triggered, window, [callbacks]() {
        if (callbacks.clearDiagnostics) {
            callbacks.clearDiagnostics();
        }
    });

    return actions;
}
