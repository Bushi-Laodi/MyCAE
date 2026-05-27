#pragma once

#include "commands/UndoStackController.h"
#include "geometry/GeometryManager.h"
#include "diagnostics/DiagnosticCollector.h"
#include "picking/PickController.h"
#include "project/ProjectModel.h"
#include "project/ProjectManager.h"
#include "result/ResultAnimationController.h"
#include "solver/plugin/SolverPluginManager.h"
#include "ui/ActionRegistry.h"
#include "ui/AppSettings.h"
#include "ui/MainWindowActions.h"
#include "ui/MainWindowDocks.h"

#include <QMainWindow>
#include <QStringList>

class QAction;
class QCloseEvent;
class QString;

class LogPanel;
class DiagnosticPanel;
class ProjectTreePanel;
class PropertyPanel;
class RenderView;
class ResultPostprocessPanel;
class RecentProjectController;
class MainWindowToolController;
class SelectionInteractionController;
class FaceGroupRestoreController;
class MainWindowResultController;
class MainWindowViewController;
struct PickSelection;
struct ResultProbe;
struct Selection;
struct WorkflowCommandContext;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool openProjectFileForAutomation(const QString &projectFilePath);
    void prepareForAutomationShutdown();

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createDockWidgets();
    void installLifecycle();

    // Delayed initialization: VTK OpenGL context may not be ready
    // during the constructor. We defer the first Render() call
    // to the first showEvent to avoid a crash in MSVCP140.dll.
    void initializeVtkRender();
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool m_vtkInitialized = false;

    void applySelection(const Selection &selection);
    void setSelectedResultField(const QString &fieldName);
    void setSelectedResultDeformationScale(double scale);
    void setSelectedResultMeshEdges(bool enabled);
    void setSelectedResultUndeformedOverlay(bool enabled);
    void setSelectedResultScalarRangeLock(bool locked);
    void setSelectedResultScalarRange(double minimum, double maximum);
    void playSelectedResultAnimation(double speed);
    void stopSelectedResultAnimation();
    void applyAnimatedResultDeformationScale(double scale);
    void exportSelectedResultCsv();
    void exportSelectedResultReport();
    void exportRenderScreenshot();
    void openSelectedResultDirectory();
    void renameSelectedResult();
    void deleteSelectedResultHistory();
    void showProjectResources();
    void openRecentProject(const QString &projectFilePath);
    void updateRecentProjectActions();
    void clearRecentProjects();
    void clearDiagnostics();
    void validateSamples();
    void refreshDiagnosticsPanel();
    void refreshResultViews();
    void handleUndoStackFaceGroupsChanged(const QString &selectionId);
    void handleFacePicked(const PickSelection &selection);
    void handleResultProbePicked(const ResultProbe &probe);
    void updateActionStates();
    WorkflowCommandContext workflowCommandContext();
    RecentProjectController recentProjectController();
    MainWindowToolController toolController();
    SelectionInteractionController selectionInteractionController();
    FaceGroupRestoreController faceGroupRestoreController();
    MainWindowResultController resultController();
    MainWindowViewController viewController();
    void writeLog(const QString &message);
    void writeLogMessages(const QStringList &messages);

    MainWindowActions m_actions;

    MainWindowDockWidgets m_docks;

    ProjectManager m_projectManager;
    GeometryManager m_geometryManager;
    ProjectModel m_projectModel;
    SolverPluginManager m_solverPluginManager;
    ActionRegistry m_actionRegistry;
    PickController m_pickController;
    DiagnosticCollector m_diagnosticCollector;
    UndoStackController m_undoStackController;
    AppSettings m_appSettings;
    QString m_activeProjectFile;
    ResultAnimationController m_resultAnimationController;
};
