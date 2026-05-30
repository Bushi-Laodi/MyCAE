#include "MainWindow.h"

#include "RenderView.h"
#include "commands/WorkflowCommandContext.h"
#include "mesh/MeshObject.h"
#include "ui/FaceGroupRestoreController.h"
#include "ui/MainWindowDocks.h"
#include "ui/MainWindowLifecycleController.h"
#include "ui/MainWindowMenuBuilder.h"
#include "ui/MainWindowResultController.h"
#include "ui/MainWindowStateController.h"
#include "ui/MainWindowToolController.h"
#include "ui/MainWindowToolBarBuilder.h"
#include "ui/MainWindowViewController.h"
#include "ui/PropertyPanel.h"
#include "ui/RecentProjectController.h"
#include "ui/RenderSettingsPanel.h"
#include "ui/ResultPostprocessPanel.h"
#include "ui/SelectionInteractionController.h"

#include <QCloseEvent>
#include <QShowEvent>
#include <QStatusBar>

#include <algorithm>
#include <vector>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

void appendElementIds(std::vector<int> &target, const QStringList &source)
{
    for (const QString &text : source) {
        bool ok = false;
        const int elementId = text.toInt(&ok);
        if (ok) {
            target.push_back(elementId);
        }
    }
}

std::vector<int> meshQualityIssueElementIds(const MeshObject &meshObject)
{
    std::vector<int> elementIds;
    elementIds.reserve(
        meshObject.invalidElementIds.size()
        + meshObject.degenerateElementIds.size()
        + meshObject.highAspectRatioElementIds.size()
    );
    appendElementIds(elementIds, meshObject.invalidElementIds);
    appendElementIds(elementIds, meshObject.degenerateElementIds);
    appendElementIds(elementIds, meshObject.highAspectRatioElementIds);
    std::sort(elementIds.begin(), elementIds.end());
    elementIds.erase(std::unique(elementIds.begin(), elementIds.end()), elementIds.end());
    return elementIds;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1200, 760);
    setWindowTitle("MyCAE - Qt 6");

    createDockWidgets();

    createActions();
    createMenus();
    createToolBar();
    installLifecycle();

    // NOTE: Do NOT call RenderView::showEmpty() here!
    // The VTK OpenGL context is not fully ready during widget construction.
    // The first Render() call is deferred to showEvent() to avoid a crash
    // in MSVCP140.dll (0xc0000005 - access violation).

    statusBar()->showMessage(zh(u8"就绪"));
    writeLog(zh(u8"MyCAE 已启动。"));
    writeLogMessages(m_solverPluginManager.diagnostics());
}

void MainWindow::prepareForAutomationShutdown()
{
    m_resultAnimationController.stop();
    m_actionRegistry.setAfterExecuteCallback({});
    m_undoStackController.setFaceGroupRestoreCallback({});
    if (m_docks.renderView) {
        m_docks.renderView->disconnect();
    }
    if (m_docks.resultPostprocessPanel) {
        m_docks.resultPostprocessPanel->disconnect();
    }
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
    if (m_docks.renderView) {
        m_docks.renderView->showEmpty();
    }
    m_vtkInitialized = true;
    writeLog(zh(u8"VTK 渲染器已初始化。"));
}

void MainWindow::createActions()
{
    MainWindowActionCallbacks callbacks;
    callbacks.openRecentProject = [this](const QString &path) { openRecentProject(path); };
    callbacks.clearRecentProjects = [this]() { clearRecentProjects(); };
    callbacks.setSelectedResultField = [this](const QString &field) { setSelectedResultField(field); };
    callbacks.setSelectedResultDeformationScale = [this](double scale) { setSelectedResultDeformationScale(scale); };
    callbacks.exportRenderScreenshot = [this]() { exportRenderScreenshot(); };
    callbacks.showProjectResources = [this]() { showProjectResources(); };
    callbacks.validateSamples = [this]() { validateSamples(); };
    callbacks.clearDiagnostics = [this]() { clearDiagnostics(); };

    m_actions = MainWindowActionBuilder::build(
        this,
        m_actionRegistry,
        workflowCommandContext(),
        m_undoStackController,
        callbacks
    );
}

void MainWindow::createMenus()
{
    MainWindowMenuBuilder::build(
        this,
        m_actions,
        m_actionRegistry,
        workflowCommandContext(),
        m_solverPluginManager,
        m_docks.logPanel
    );
}

void MainWindow::createToolBar()
{
    MainWindowToolBarBuilder::build(this, m_actions);
}

void MainWindow::createDockWidgets()
{
    MainWindowDockCallbacks callbacks;
    callbacks.facePicked = [this](const PickSelection &selection) { handleFacePicked(selection); };
    callbacks.resultProbePicked = [this](const ResultProbe &probe) { handleResultProbePicked(probe); };
    callbacks.selectionChanged = [this](const Selection &selection) { applySelection(selection); };
    callbacks.resultFieldChanged = [this](const QString &field) { setSelectedResultField(field); };
    callbacks.resultDeformationScaleChanged = [this](double scale) { setSelectedResultDeformationScale(scale); };
    callbacks.resultMeshEdgesChanged = [this](bool enabled) { setSelectedResultMeshEdges(enabled); };
    callbacks.resultUndeformedOverlayChanged = [this](bool enabled) { setSelectedResultUndeformedOverlay(enabled); };
    callbacks.resultScalarRangeLockChanged = [this](bool locked) { setSelectedResultScalarRangeLock(locked); };
    callbacks.resultScalarRangeChanged = [this](double minimum, double maximum) {
        setSelectedResultScalarRange(minimum, maximum);
    };
    callbacks.resultAnimationPlayRequested = [this](double speed) { playSelectedResultAnimation(speed); };
    callbacks.resultAnimationStopRequested = [this]() { stopSelectedResultAnimation(); };
    callbacks.resultExportCsvRequested = [this]() { exportSelectedResultCsv(); };
    callbacks.resultExportReportRequested = [this]() { exportSelectedResultReport(); };
    callbacks.resultExportScreenshotRequested = [this]() { exportRenderScreenshot(); };
    callbacks.resultOpenDirectoryRequested = [this]() { openSelectedResultDirectory(); };
    callbacks.resultRenameRequested = [this]() { renameSelectedResult(); };
    callbacks.resultDeleteRequested = [this]() { deleteSelectedResultHistory(); };

    m_docks = MainWindowDockBuilder::build(this, callbacks);
    if (m_docks.propertyPanel && m_docks.renderView) {
        m_docks.propertyPanel->setMeshCallbacks(PropertyPanelMeshCallbacks{
            [this](const MeshObject &meshObject) {
                const std::vector<int> elementIds = meshQualityIssueElementIds(meshObject);
                if (elementIds.empty()) {
                    writeLog(zh(u8"当前网格没有可高亮的质量问题单元。"));
                    return;
                }
                m_docks.renderView->highlightMeshElements(elementIds);
                writeLog(zh(u8"已高亮网格质量问题单元：%1 个").arg(static_cast<int>(elementIds.size())));
            },
            [this]() {
                if (m_docks.renderView) {
                    m_docks.renderView->clearHighlight();
                }
            }
        });
    }
    if (m_docks.renderSettingsPanel && m_docks.renderView) {
        m_docks.renderSettingsPanel->setSettings(m_docks.renderView->renderDisplaySettings());
        connect(
            m_docks.renderSettingsPanel,
            &RenderSettingsPanel::settingsChanged,
            this,
            [this](const RenderDisplaySettings &settings) {
                applyRenderDisplaySettings(settings);
            }
        );
        connect(
            m_docks.renderSettingsPanel,
            &RenderSettingsPanel::resetRequested,
            this,
            [this]() {
                resetRenderDisplaySettings();
            }
        );
    }
}

void MainWindow::installLifecycle()
{
    MainWindowLifecycleCallbacks callbacks;
    callbacks.faceGroupRestored = [this](const QString &selectionId) {
        handleUndoStackFaceGroupsChanged(selectionId);
    };
    callbacks.resultAnimationFrameScaleChanged = [this](double scale) {
        applyAnimatedResultDeformationScale(scale);
    };
    callbacks.updateRecentProjects = [this]() { updateRecentProjectActions(); };
    callbacks.updateActionStates = [this]() { updateActionStates(); };

    MainWindowLifecycleController::install(
        MainWindowLifecycleContext{
            *this,
            m_actionRegistry,
            m_undoStackController,
            m_appSettings,
            m_projectModel,
            m_resultAnimationController,
            m_docks.resultPostprocessPanel,
            m_activeProjectFile
        },
        callbacks
    );
}

void MainWindow::applySelection(const Selection &selection)
{
    selectionInteractionController().applySelection(selection);
}

void MainWindow::setSelectedResultField(const QString &fieldName)
{
    resultController().setSelectedField(fieldName);
}

void MainWindow::setSelectedResultDeformationScale(double scale)
{
    resultController().setSelectedDeformationScale(scale);
}

void MainWindow::setSelectedResultMeshEdges(bool enabled)
{
    resultController().setSelectedMeshEdges(enabled);
}

void MainWindow::setSelectedResultUndeformedOverlay(bool enabled)
{
    resultController().setSelectedUndeformedOverlay(enabled);
}

void MainWindow::setSelectedResultScalarRangeLock(bool locked)
{
    resultController().setSelectedScalarRangeLock(locked);
}

void MainWindow::setSelectedResultScalarRange(double minimum, double maximum)
{
    resultController().setSelectedScalarRange(minimum, maximum);
}

void MainWindow::playSelectedResultAnimation(double speed)
{
    resultController().playSelectedAnimation(speed);
}

void MainWindow::stopSelectedResultAnimation()
{
    resultController().stopSelectedAnimation();
}

void MainWindow::applyAnimatedResultDeformationScale(double scale)
{
    resultController().applyAnimatedDeformationScale(scale);
}

void MainWindow::showProjectResources()
{
    toolController().showProjectResources();
}

void MainWindow::openRecentProject(const QString &projectFilePath)
{
    recentProjectController().openProject(projectFilePath);
}

bool MainWindow::openProjectFileForAutomation(const QString &projectFilePath)
{
    recentProjectController().openProject(projectFilePath);
    return m_projectModel.hasProject();
}

void MainWindow::updateRecentProjectActions()
{
    recentProjectController().updateActions();
}

void MainWindow::clearRecentProjects()
{
    recentProjectController().clearProjects();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    MainWindowLifecycleController::saveWindowState(m_appSettings, *this);
    QMainWindow::closeEvent(event);
}

void MainWindow::clearDiagnostics()
{
    toolController().clearDiagnostics();
}

void MainWindow::validateSamples()
{
    toolController().validateSamples();
}

void MainWindow::refreshDiagnosticsPanel()
{
    viewController().refreshDiagnosticsPanel();
}

void MainWindow::refreshResultViews()
{
    viewController().refreshResultViews();
}

void MainWindow::applyRenderDisplaySettings(const RenderDisplaySettings &settings)
{
    if (m_docks.renderView) {
        m_docks.renderView->setRenderDisplaySettings(settings);
    }
    updateActionStates();
}

void MainWindow::resetRenderDisplaySettings()
{
    const RenderDisplaySettings defaults;
    if (m_docks.renderView) {
        m_docks.renderView->setRenderDisplaySettings(defaults);
    }
    if (m_docks.renderSettingsPanel) {
        m_docks.renderSettingsPanel->setSettings(defaults);
    }
    updateActionStates();
}

void MainWindow::syncRenderSettingsPanel()
{
    if (m_docks.renderSettingsPanel && m_docks.renderView) {
        m_docks.renderSettingsPanel->setSettings(m_docks.renderView->renderDisplaySettings());
    }
}

void MainWindow::handleUndoStackFaceGroupsChanged(const QString &selectionId)
{
    faceGroupRestoreController().restoreAfterUndoStackChange(selectionId);
}

void MainWindow::exportSelectedResultCsv()
{
    resultController().exportSelectedCsv();
}

void MainWindow::exportSelectedResultReport()
{
    resultController().exportSelectedReport();
}

void MainWindow::exportRenderScreenshot()
{
    resultController().exportRenderScreenshot();
}

void MainWindow::openSelectedResultDirectory()
{
    resultController().openSelectedResultDirectory();
}

void MainWindow::renameSelectedResult()
{
    resultController().renameSelectedResult();
}

void MainWindow::deleteSelectedResultHistory()
{
    resultController().deleteSelectedResultHistory();
}

void MainWindow::handleFacePicked(const PickSelection &selection)
{
    selectionInteractionController().handleFacePicked(selection);
}

void MainWindow::handleResultProbePicked(const ResultProbe &probe)
{
    resultController().setSelectedProbe(probe);
}

void MainWindow::updateActionStates()
{
    MainWindowStateController::update(
        m_actions,
        m_projectModel,
        m_pickController,
        m_diagnosticCollector,
        m_docks.renderView,
        m_docks.resultPostprocessPanel
    );
    syncRenderSettingsPanel();
}

WorkflowCommandContext MainWindow::workflowCommandContext()
{
    return WorkflowCommandContext{
        m_projectManager,
        m_geometryManager,
        m_projectModel,
        m_solverPluginManager,
        m_docks.projectTreePanel,
        m_docks.propertyPanel,
        m_docks.renderView,
        m_docks.logPanel,
        this,
        &m_pickController,
        &m_undoStackController,
        [this](const QStringList &messages) { writeLogMessages(messages); }
    };
}

RecentProjectController MainWindow::recentProjectController()
{
    RecentProjectCallbacks callbacks;
    callbacks.writeLog = [this](const QString &message) { writeLog(message); };
    callbacks.writeLogMessages = [this](const QStringList &messages) { writeLogMessages(messages); };
    callbacks.updateActionStates = [this]() { updateActionStates(); };

    return RecentProjectController(
        RecentProjectContext{
            m_projectManager,
            m_geometryManager,
            m_projectModel,
            m_actions,
            m_docks,
            m_undoStackController,
            m_appSettings,
            m_activeProjectFile,
            this
        },
        callbacks
    );
}

MainWindowToolController MainWindow::toolController()
{
    MainWindowToolCallbacks callbacks;
    callbacks.writeLog = [this](const QString &message) { writeLog(message); };
    callbacks.writeLogMessages = [this](const QStringList &messages) { writeLogMessages(messages); };
    callbacks.refreshDiagnosticsPanel = [this]() { refreshDiagnosticsPanel(); };
    callbacks.refreshResultViews = [this]() { refreshResultViews(); };

    return MainWindowToolController(
        MainWindowToolContext{
            m_projectModel,
            m_docks,
            m_diagnosticCollector,
            this
        },
        callbacks
    );
}

SelectionInteractionController MainWindow::selectionInteractionController()
{
    SelectionInteractionCallbacks callbacks;
    callbacks.stopResultAnimation = [this]() { stopSelectedResultAnimation(); };
    callbacks.writeLogMessages = [this](const QStringList &messages) { writeLogMessages(messages); };
    callbacks.showStatusMessage = [this](const QString &message) { statusBar()->showMessage(message); };
    callbacks.updateActionStates = [this]() { updateActionStates(); };

    return SelectionInteractionController(
        SelectionInteractionContext{
            m_projectModel,
            m_pickController,
            m_resultAnimationController,
            m_docks
        },
        callbacks
    );
}

FaceGroupRestoreController MainWindow::faceGroupRestoreController()
{
    FaceGroupRestoreCallbacks callbacks;
    callbacks.writeLogMessages = [this](const QStringList &messages) { writeLogMessages(messages); };
    callbacks.applySelection = [this](const Selection &selection) { applySelection(selection); };
    callbacks.updateActionStates = [this]() { updateActionStates(); };

    return FaceGroupRestoreController(
        FaceGroupRestoreContext{
            m_projectManager,
            m_geometryManager,
            m_projectModel,
            m_docks,
            this
        },
        callbacks
    );
}

MainWindowResultController MainWindow::resultController()
{
    MainWindowResultCallbacks callbacks;
    callbacks.writeLogMessages = [this](const QStringList &messages) { writeLogMessages(messages); };
    callbacks.updateActionStates = [this]() { updateActionStates(); };

    return MainWindowResultController(
        MainWindowResultContext{
            m_projectModel,
            m_docks,
            m_appSettings,
            m_resultAnimationController,
            this
        },
        callbacks
    );
}

MainWindowViewController MainWindow::viewController()
{
    MainWindowViewCallbacks callbacks;
    callbacks.updateActionStates = [this]() { updateActionStates(); };

    return MainWindowViewController(
        MainWindowViewContext{
            m_projectModel,
            m_docks,
            m_diagnosticCollector
        },
        callbacks
    );
}

void MainWindow::writeLog(const QString &message)
{
    viewController().writeLog(message);
}

void MainWindow::writeLogMessages(const QStringList &messages)
{
    viewController().writeLogMessages(messages);
}
