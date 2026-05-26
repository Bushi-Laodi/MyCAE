#include "ui/MainWindowMenuBuilder.h"

#include "commands/SolverCommands.h"
#include "commands/UtilityCommands.h"
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
constexpr auto CommandImportStep = "geometry.import.step";

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
    auto *fileMenu = window->menuBar()->addMenu("File");
    fileMenu->addAction(actions.newProject);
    fileMenu->addAction(actions.openProject);
    actions.recentProjectsMenu = fileMenu->addMenu("Recent Projects");
    for (QAction *recentAction : actions.recentProjects) {
        actions.recentProjectsMenu->addAction(recentAction);
    }
    actions.recentProjectsMenu->addSeparator();
    actions.recentProjectsMenu->addAction(actions.clearRecentProjects);
    fileMenu->addSeparator();
    fileMenu->addAction(actions.exit);

    auto *editMenu = window->menuBar()->addMenu("Edit");
    editMenu->addAction(actions.undo);
    editMenu->addAction(actions.redo);

    auto *geometryMenu = window->menuBar()->addMenu("Geometry");
    geometryMenu->addAction(actions.createBox);
    geometryMenu->addAction(actions.createCylinder);
    QAction *importStepAction = geometryMenu->addAction("Import STEP");
    actionRegistry.registerActionCommand(
        CommandImportStep,
        importStepAction,
        window,
        makeLogMessageCommand(logPanel, "STEP import is reserved for a later stage.")
    );
    geometryMenu->addSeparator();
    geometryMenu->addAction(actions.pickFace);
    geometryMenu->addAction(actions.clearPick);
    auto *faceGroupMenu = geometryMenu->addMenu("Face Groups");
    faceGroupMenu->addAction(actions.createFaceGroupFromPick);
    faceGroupMenu->addAction(actions.addPickedFacesToFaceGroup);
    faceGroupMenu->addAction(actions.removePickedFacesFromFaceGroup);
    faceGroupMenu->addAction(actions.clearFaceGroupFaces);
    faceGroupMenu->addSeparator();
    faceGroupMenu->addAction(actions.renameFaceGroup);
    faceGroupMenu->addAction(actions.deleteFaceGroup);
    faceGroupMenu->addSeparator();
    faceGroupMenu->addAction(actions.setFaceGroupLocalMeshSize);
    faceGroupMenu->addAction(actions.toggleFaceGroupPhysicalGroup);

    auto *caseMenu = window->menuBar()->addMenu("Case");
    caseMenu->addAction(actions.createMaterial);
    caseMenu->addAction(actions.createBoundaryCondition);
    caseMenu->addAction(actions.createLoad);
    caseMenu->addSeparator();
    caseMenu->addAction(actions.editSolverData);
    caseMenu->addAction(actions.deleteSolverData);

    auto *simulationMenu = window->menuBar()->addMenu("Simulation");
    simulationMenu->addAction(actions.checkGmsh);
    simulationMenu->addAction(actions.generateMesh);
    simulationMenu->addAction(actions.readMeshInfo);
    simulationMenu->addAction(actions.showMesh);
    simulationMenu->addSeparator();
    if (solverPluginManager.pluginDescriptors().empty()) {
        QAction *noSolverAction = simulationMenu->addAction("No solver plugins found");
        noSolverAction->setEnabled(false);
    } else {
        for (const SolverPluginDescriptor &descriptor : solverPluginManager.pluginDescriptors()) {
            const QString pluginId = descriptor.id;
            QAction *runSolverAction =
                simulationMenu->addAction(SolverPluginDescriptorFormatter::menuText(descriptor));
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

    auto *resultsMenu = window->menuBar()->addMenu("Results");
    auto *fieldMenu = resultsMenu->addMenu("Result Field");
    for (QAction *fieldAction : actions.resultFields) {
        fieldMenu->addAction(fieldAction);
    }
    auto *scaleMenu = resultsMenu->addMenu("Deformation Scale");
    for (QAction *scaleAction : actions.resultScales) {
        scaleMenu->addAction(scaleAction);
    }
    resultsMenu->addSeparator();
    resultsMenu->addAction(actions.exportScreenshot);

    auto *toolsMenu = window->menuBar()->addMenu("Tools");
    toolsMenu->addAction(actions.projectResources);
    toolsMenu->addAction(actions.validateSamples);
    toolsMenu->addSeparator();
    toolsMenu->addAction(actions.clearDiagnostics);
}
