#include "ui/MainWindowMenuBuilder.h"

#include "commands/SolverCommands.h"
#include "commands/WorkflowCommandContext.h"
#include "solver/plugin/SolverPluginDescriptorFormatter.h"
#include "solver/plugin/SolverPluginManager.h"
#include "ui/ActionRegistry.h"
#include "ui/MainWindowActions.h"

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QString>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString solverRunCommandId(const QString &pluginId)
{
    return "solver.run." + pluginId;
}
}

void MainWindowMenuBuilder::build(
    QMainWindow *window,
    MainWindowActions &actions,
    ActionRegistry &actionRegistry,
    const WorkflowCommandContext &context,
    const SolverPluginManager &solverPluginManager,
    LogPanel *logPanel
)
{
    Q_UNUSED(logPanel);

    auto *fileMenu = window->menuBar()->addMenu(zh(u8"文件"));
    fileMenu->addAction(actions.newProject);
    fileMenu->addAction(actions.openProject);
    actions.recentProjectsMenu = fileMenu->addMenu(zh(u8"最近工程"));
    for (QAction *recentAction : actions.recentProjects) {
        actions.recentProjectsMenu->addAction(recentAction);
    }
    actions.recentProjectsMenu->addSeparator();
    actions.recentProjectsMenu->addAction(actions.clearRecentProjects);
    fileMenu->addSeparator();
    fileMenu->addAction(actions.projectResources);
    fileMenu->addAction(actions.exportScreenshot);
    fileMenu->addSeparator();
    fileMenu->addAction(actions.exit);

    auto *editMenu = window->menuBar()->addMenu(zh(u8"编辑"));
    editMenu->addAction(actions.undo);
    editMenu->addAction(actions.redo);

    auto *viewMenu = window->menuBar()->addMenu(zh(u8"视图"));
    viewMenu->addAction(actions.showGeometryEdges);
    viewMenu->addAction(actions.showOrientationMarker);
    viewMenu->addAction(actions.showMeshTransparent);
    viewMenu->addSeparator();
    viewMenu->addAction(actions.clearDiagnostics);

    auto *geometryMenu = window->menuBar()->addMenu(zh(u8"几何"));
    geometryMenu->addAction(actions.createBox);
    geometryMenu->addAction(actions.createCylinder);
    geometryMenu->addAction(actions.createSphere);
    geometryMenu->addAction(actions.importStep);
    geometryMenu->addSeparator();
    geometryMenu->addAction(actions.createBoolean);
    geometryMenu->addAction(actions.transformGeometry);
    geometryMenu->addAction(actions.deleteGeometry);

    auto *selectionMenu = window->menuBar()->addMenu(zh(u8"选择与分组"));
    selectionMenu->addAction(actions.pickFace);
    selectionMenu->addAction(actions.clearPick);
    selectionMenu->addSeparator();
    selectionMenu->addAction(actions.createFaceGroupFromPick);
    selectionMenu->addAction(actions.addPickedFacesToFaceGroup);
    selectionMenu->addAction(actions.removePickedFacesFromFaceGroup);
    selectionMenu->addAction(actions.clearFaceGroupFaces);
    selectionMenu->addSeparator();
    selectionMenu->addAction(actions.renameFaceGroup);
    selectionMenu->addAction(actions.deleteFaceGroup);
    selectionMenu->addSeparator();
    selectionMenu->addAction(actions.setFaceGroupLocalMeshSize);
    selectionMenu->addAction(actions.toggleFaceGroupPhysicalGroup);

    auto *meshMenu = window->menuBar()->addMenu(zh(u8"网格"));
    meshMenu->addAction(actions.checkGmsh);
    meshMenu->addAction(actions.generateMesh);
    meshMenu->addAction(actions.readMeshInfo);
    meshMenu->addAction(actions.showMesh);

    auto *caseMenu = window->menuBar()->addMenu(zh(u8"仿真设置"));
    auto *structuralCaseMenu = caseMenu->addMenu(zh(u8"结构工况"));
    structuralCaseMenu->addAction(actions.createStructuralMaterial);
    structuralCaseMenu->addAction(actions.createSectionAssignment);
    structuralCaseMenu->addAction(actions.createStructuralBoundaryCondition);
    structuralCaseMenu->addAction(actions.createStructuralLoad);
    auto *cfdCaseMenu = caseMenu->addMenu(zh(u8"CFD 工况"));
    cfdCaseMenu->addAction(actions.createFluidMaterial);
    cfdCaseMenu->addAction(actions.createCfdBoundaryCondition);
    cfdCaseMenu->addAction(actions.createCfdFieldValue);
    caseMenu->addSeparator();
    caseMenu->addAction(actions.editSolverData);
    caseMenu->addAction(actions.deleteSolverData);

    auto *solverMenu = window->menuBar()->addMenu(zh(u8"求解"));
    if (solverPluginManager.pluginDescriptors().empty()) {
        QAction *noSolverAction = solverMenu->addAction(zh(u8"未找到求解器插件"));
        noSolverAction->setEnabled(false);
    } else {
        for (const SolverPluginDescriptor &descriptor : solverPluginManager.pluginDescriptors()) {
            const QString pluginId = descriptor.id;
            QAction *runSolverAction =
                solverMenu->addAction(SolverPluginDescriptorFormatter::menuText(descriptor));
            runSolverAction->setProperty("solverUsable", descriptor.isUsable());
            actions.runSolvers.append(runSolverAction);
            runSolverAction->setStatusTip(SolverPluginDescriptorFormatter::statusTip(descriptor));
            actionRegistry.registerActionCommand(
                solverRunCommandId(pluginId),
                runSolverAction,
                window,
                makeRunSolverCommand(context, pluginId)
            );
        }
    }

    auto *resultsMenu = window->menuBar()->addMenu(zh(u8"结果"));
    auto *fieldMenu = resultsMenu->addMenu(zh(u8"结果场"));
    for (QAction *fieldAction : actions.resultFields) {
        fieldMenu->addAction(fieldAction);
    }
    auto *scaleMenu = resultsMenu->addMenu(zh(u8"变形比例"));
    for (QAction *scaleAction : actions.resultScales) {
        scaleMenu->addAction(scaleAction);
    }
    resultsMenu->addSeparator();
    resultsMenu->addAction(actions.exportScreenshot);

    auto *toolsMenu = window->menuBar()->addMenu(zh(u8"工具"));
    toolsMenu->addAction(actions.validateSamples);
}
