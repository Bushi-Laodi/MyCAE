#include "ui/MainWindowDocks.h"

#include "ui/DiagnosticPanel.h"
#include "ui/LogPanel.h"
#include "ui/ProjectTreePanel.h"
#include "ui/PropertyPanel.h"
#include "ui/RenderView.h"
#include "ui/RenderSettingsPanel.h"
#include "ui/ResultPostprocessPanel.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QScrollArea>
#include <QSizePolicy>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QScrollArea *createDockScrollArea(QWidget *content, QWidget *parent, const QString &objectName, int minimumWidth)
{
    auto *scrollArea = new QScrollArea(parent);
    scrollArea->setObjectName(objectName);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setMinimumWidth(minimumWidth);
    content->setMinimumWidth(minimumWidth - 18);
    content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    scrollArea->setWidget(content);
    return scrollArea;
}
}

MainWindowDockWidgets MainWindowDockBuilder::build(QMainWindow *window, const MainWindowDockCallbacks &callbacks)
{
    MainWindowDockWidgets widgets;

    widgets.renderView = new RenderView(window);
    QObject::connect(widgets.renderView, &RenderView::facePicked, window, [callbacks](const PickSelection &selection) {
        if (callbacks.facePicked) {
            callbacks.facePicked(selection);
        }
    });
    QObject::connect(widgets.renderView, &RenderView::resultProbePicked, window, [callbacks](const ResultProbe &probe) {
        if (callbacks.resultProbePicked) {
            callbacks.resultProbePicked(probe);
        }
    });
    window->setCentralWidget(widgets.renderView);

    auto *projectDock = new QDockWidget(zh(u8"工程 / 模型"), window);
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

    auto *diagnosticDock = new QDockWidget(zh(u8"诊断"), window);
    widgets.diagnosticPanel = new DiagnosticPanel(diagnosticDock);
    diagnosticDock->setWidget(widgets.diagnosticPanel);
    window->addDockWidget(Qt::BottomDockWidgetArea, diagnosticDock);

    auto *propertyDock = new QDockWidget(zh(u8"属性"), window);
    propertyDock->setMinimumWidth(340);
    widgets.propertyPanel = new PropertyPanel(propertyDock);
    propertyDock->setWidget(createDockScrollArea(widgets.propertyPanel, propertyDock, "property.scrollArea", 340));
    window->addDockWidget(Qt::RightDockWidgetArea, propertyDock);

    auto *postprocessDock = new QDockWidget(zh(u8"结果后处理"), window);
    postprocessDock->setMinimumWidth(380);
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
        &ResultPostprocessPanel::scalarRangeLockChanged,
        window,
        [callbacks](bool locked) {
            if (callbacks.resultScalarRangeLockChanged) {
                callbacks.resultScalarRangeLockChanged(locked);
            }
        }
    );
    QObject::connect(
        widgets.resultPostprocessPanel,
        &ResultPostprocessPanel::scalarRangeChanged,
        window,
        [callbacks](double minimum, double maximum) {
            if (callbacks.resultScalarRangeChanged) {
                callbacks.resultScalarRangeChanged(minimum, maximum);
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
    postprocessDock->setWidget(
        createDockScrollArea(widgets.resultPostprocessPanel, postprocessDock, "resultPostprocess.scrollArea", 380)
    );
    window->addDockWidget(Qt::RightDockWidgetArea, postprocessDock);
    window->tabifyDockWidget(propertyDock, postprocessDock);

    auto *renderSettingsDock = new QDockWidget(zh(u8"渲染设置"), window);
    renderSettingsDock->setMinimumWidth(300);
    widgets.renderSettingsPanel = new RenderSettingsPanel(renderSettingsDock);
    renderSettingsDock->setWidget(
        createDockScrollArea(widgets.renderSettingsPanel, renderSettingsDock, "renderSettings.scrollArea", 300)
    );
    window->addDockWidget(Qt::RightDockWidgetArea, renderSettingsDock);
    window->tabifyDockWidget(propertyDock, renderSettingsDock);

    auto *logDock = new QDockWidget(zh(u8"日志"), window);
    widgets.logPanel = new LogPanel(logDock);
    logDock->setWidget(widgets.logPanel);
    window->addDockWidget(Qt::BottomDockWidgetArea, logDock);
    window->tabifyDockWidget(logDock, diagnosticDock);

    return widgets;
}
