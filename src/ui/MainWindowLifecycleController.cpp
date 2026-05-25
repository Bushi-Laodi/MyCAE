#include "ui/MainWindowLifecycleController.h"

#include "commands/UndoStackController.h"
#include "project/ProjectModel.h"
#include "result/ResultAnimationController.h"
#include "ui/ActionRegistry.h"
#include "ui/AppSettings.h"
#include "ui/ResultPostprocessPanel.h"

#include <QMainWindow>

void MainWindowLifecycleController::install(
    const MainWindowLifecycleContext &context,
    const MainWindowLifecycleCallbacks &callbacks
)
{
    context.undoStackController.setFaceGroupRestoreCallback([callbacks](const QString &selectionId) {
        if (callbacks.faceGroupRestored) {
            callbacks.faceGroupRestored(selectionId);
        }
    });

    context.actionRegistry.setAfterExecuteCallback([context, callbacks]() {
        if (context.projectModel.hasProject()) {
            if (context.activeProjectFile != context.projectModel.project().projectFilePath) {
                context.undoStackController.clear();
                context.activeProjectFile = context.projectModel.project().projectFilePath;
            }
            context.appSettings.addRecentProject(context.projectModel.project().projectFilePath);
            if (callbacks.updateRecentProjects) {
                callbacks.updateRecentProjects();
            }
        }
        if (context.resultPostprocessPanel) {
            context.resultPostprocessPanel->setResult(context.projectModel.resultForSelection());
        }
        if (callbacks.updateActionStates) {
            callbacks.updateActionStates();
        }
    });

    QObject::connect(
        &context.resultAnimationController,
        &ResultAnimationController::frameScaleChanged,
        &context.window,
        [callbacks](double scale) {
            if (callbacks.resultAnimationFrameScaleChanged) {
                callbacks.resultAnimationFrameScaleChanged(scale);
            }
        }
    );

    if (callbacks.updateActionStates) {
        callbacks.updateActionStates();
    }
    if (callbacks.updateRecentProjects) {
        callbacks.updateRecentProjects();
    }
    context.appSettings.restoreMainWindow(context.window);
}

void MainWindowLifecycleController::saveWindowState(AppSettings &appSettings, QMainWindow &window)
{
    appSettings.saveMainWindow(window);
}
