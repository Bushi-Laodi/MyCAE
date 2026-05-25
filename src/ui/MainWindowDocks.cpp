#include "ui/MainWindowDocks.h"

#include "ui/DiagnosticPanel.h"
#include "ui/LogPanel.h"
#include "ui/ProjectTreePanel.h"
#include "ui/PropertyPanel.h"
#include "ui/RenderView.h"
#include "ui/ResultPostprocessPanel.h"

#include <QDockWidget>
#include <QMainWindow>

MainWindowDockWidgets MainWindowDockBuilder::build(QMainWindow *window, const MainWindowDockCallbacks &callbacks)
{
    MainWindowDockWidgets widgets;

    widgets.renderView = new RenderView(window);
    QObject::connect(widgets.renderView, &RenderView::facePicked, window, [callbacks](const PickSelection &selection) {
        if (callbacks.facePicked) {
            callbacks.facePicked(selection);
        }
    });
    window->setCentralWidget(widgets.renderView);

    auto *projectDock = new QDockWidget("Project / Model", window);
    widgets.projectTreePanel = new ProjectTreePanel(projectDock);
    QObject::connect(
        widgets.projectTreePanel,
        &ProjectTreePanel::selectionChanged,
        window,
        [callbacks](const Selection &selection) {
            if (callbacks.selectionChanged) {
                callbacks.selectionChanged(selection);
            }
        }
    );
    projectDock->setWidget(widgets.projectTreePanel);
    window->addDockWidget(Qt::LeftDockWidgetArea, projectDock);

    auto *diagnosticDock = new QDockWidget("Diagnostics", window);
    widgets.diagnosticPanel = new DiagnosticPanel(diagnosticDock);
    diagnosticDock->setWidget(widgets.diagnosticPanel);
    window->addDockWidget(Qt::BottomDockWidgetArea, diagnosticDock);

    auto *propertyDock = new QDockWidget("Properties", window);
    widgets.propertyPanel = new PropertyPanel(propertyDock);
    propertyDock->setWidget(widgets.propertyPanel);
    window->addDockWidget(Qt::RightDockWidgetArea, propertyDock);

    auto *postprocessDock = new QDockWidget("Result Postprocess", window);
    widgets.resultPostprocessPanel = new ResultPostprocessPanel(postprocessDock);
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::fieldChanged,
        window,
        [callbacks](const QString &fieldName) {
            if (callbacks.resultFieldChanged) {
                callbacks.resultFieldChanged(fieldName);
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::deformationScaleChanged,
        window,
        [callbacks](double scale) {
            if (callbacks.resultDeformationScaleChanged) {
                callbacks.resultDeformationScaleChanged(scale);
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::meshEdgesChanged,
        window,
        [callbacks](bool enabled) {
            if (callbacks.resultMeshEdgesChanged) {
                callbacks.resultMeshEdgesChanged(enabled);
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::undeformedOverlayChanged,
        window,
        [callbacks](bool enabled) {
            if (callbacks.resultUndeformedOverlayChanged) {
                callbacks.resultUndeformedOverlayChanged(enabled);
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::animationPlayRequested,
        window,
        [callbacks](double speed) {
            if (callbacks.resultAnimationPlayRequested) {
                callbacks.resultAnimationPlayRequested(speed);
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::animationStopRequested,
        window,
        [callbacks]() {
            if (callbacks.resultAnimationStopRequested) {
                callbacks.resultAnimationStopRequested();
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::exportCsvRequested,
        window,
        [callbacks]() {
            if (callbacks.resultExportCsvRequested) {
                callbacks.resultExportCsvRequested();
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::exportReportRequested,
        window,
        [callbacks]() {
            if (callbacks.resultExportReportRequested) {
                callbacks.resultExportReportRequested();
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::exportScreenshotRequested,
        window,
        [callbacks]() {
            if (callbacks.resultExportScreenshotRequested) {
                callbacks.resultExportScreenshotRequested();
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::openResultDirectoryRequested,
        window,
        [callbacks]() {
            if (callbacks.resultOpenDirectoryRequested) {
                callbacks.resultOpenDirectoryRequested();
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::renameResultRequested,
        window,
        [callbacks]() {
            if (callbacks.resultRenameRequested) {
                callbacks.resultRenameRequested();
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::deleteResultRequested,
        window,
        [callbacks]() {
            if (callbacks.resultDeleteRequested) {
                callbacks.resultDeleteRequested();
            }
        }
    );
    postprocessDock->setWidget(widgets.resultPostprocessPanel);
    window->addDockWidget(Qt::RightDockWidgetArea, postprocessDock);
    window->tabifyDockWidget(propertyDock, postprocessDock);

    auto *logDock = new QDockWidget("Log", window);
    widgets.logPanel = new LogPanel(logDock);
    logDock->setWidget(widgets.logPanel);
    window->addDockWidget(Qt::BottomDockWidgetArea, logDock);
    window->tabifyDockWidget(logDock, diagnosticDock);

    return widgets;
}
