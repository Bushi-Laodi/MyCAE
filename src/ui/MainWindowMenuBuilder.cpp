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
constexpr auto CommandSimulationGenerateMesh = "simulation.generateMesh";

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

    auto *pickingMenu = window->menuBar()->addMenu("Picking");
    pickingMenu->addAction(actions.pickFace);
    pickingMenu->addAction(actions.clearPick);
    pickingMenu->addSeparator();
    pickingMenu->addAction(actions.createFaceGroupFromPick);
    pickingMenu->addAction(actions.addPickedFacesToFaceGroup);
    pickingMenu->addAction(actions.removePickedFacesFromFaceGroup);
    pickingMenu->addAction(actions.clearFaceGroupFaces);
    pickingMenu->addSeparator();
    pickingMenu->addAction(actions.renameFaceGroup);
    pickingMenu->addAction(actions.deleteFaceGroup);
    pickingMenu->addSeparator();
    pickingMenu->addAction(actions.setFaceGroupLocalMeshSize);
    pickingMenu->addAction(actions.toggleFaceGroupPhysicalGroup);

    auto *solverMenu = window->menuBar()->addMenu("Solver Setup");
    solverMenu->addAction(actions.createMaterial);
    solverMenu->addAction(actions.createBoundaryCondition);
    solverMenu->addAction(actions.createLoad);
    solverMenu->addSeparator();
    solverMenu->addAction(actions.editSolverData);
    solverMenu->addAction(actions.deleteSolverData);

    auto *meshMenu = window->menuBar()->addMenu("Mesh");
    meshMenu->addAction(actions.checkGmsh);
    meshMenu->addAction(actions.generateMesh);
    meshMenu->addAction(actions.readMeshInfo);
    meshMenu->addAction(actions.showMesh);

    auto *simulationMenu = window->menuBar()->addMenu("Simulation");
    QAction *simulationGenerateMeshAction = simulationMenu->addAction("Generate Mesh");
    actionRegistry.registerActionCommand(
        CommandSimulationGenerateMesh,
        simulationGenerateMeshAction,
        window,
        makeLogMessageCommand(logPanel, "Use Mesh -> Generate Mesh for the current external Gmsh flow.")
    );

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

    auto *postMenu = window->menuBar()->addMenu("Postprocess");
    auto *fieldMenu = postMenu->addMenu("Result Field");
    for (QAction *fieldAction : actions.resultFields) {
        fieldMenu->addAction(fieldAction);
    }
    auto *scaleMenu = postMenu->addMenu("Deformation Scale");
    for (QAction *scaleAction : actions.resultScales) {
        scaleMenu->addAction(scaleAction);
    }
    postMenu->addSeparator();
    postMenu->addAction(actions.exportScreenshot);

    auto *toolsMenu = window->menuBar()->addMenu("Tools");
    toolsMenu->addAction(actions.projectResources);
    toolsMenu->addAction(actions.validateSamples);
    toolsMenu->addSeparator();
    toolsMenu->addAction(actions.clearDiagnostics);
}
