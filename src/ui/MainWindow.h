#pragma once

#include "commands/UndoStackController.h"
#include "geometry/GeometryManager.h"
#include "diagnostics/DiagnosticCollector.h"
#include "picking/PickController.h"
#include "project/ProjectModel.h"
#include "project/ProjectManager.h"
#include "project/SelectionState.h"
#include "result/ResultAnimationController.h"
#include "solver/plugin/SolverPluginManager.h"
#include "ui/ActionRegistry.h"
#include "ui/AppSettings.h"

#include <QMainWindow>
#include <QVector>
#include <QStringList>

class QAction;
class QCloseEvent;
class QMenu;
class QString;

class LogPanel;
class DiagnosticPanel;
class ProjectTreePanel;
class PropertyPanel;
class RenderView;
class ResultPostprocessPanel;
struct WorkflowCommandContext;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createDockWidgets();

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
    void playSelectedResultAnimation(double speed);
    void stopSelectedResultAnimation();
    void applyAnimatedResultDeformationScale(double scale);
    void exportSelectedResultCsv();
    void exportSelectedResultReport();
    void exportRenderScreenshot();
    void openSelectedResultDirectory();
    void renameSelectedResult();
    void deleteSelectedResultHistory();
    void redisplaySelectedResult();
    void saveResultIndex();
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
    void updateActionStates();
    WorkflowCommandContext workflowCommandContext();
    void writeLog(const QString &message);
    void writeLogMessages(const QStringList &messages);

    QAction *m_newProjectAction = nullptr;
    QAction *m_openProjectAction = nullptr;
    QMenu *m_recentProjectsMenu = nullptr;
    QVector<QAction *> m_recentProjectActions;
    QAction *m_clearRecentProjectsAction = nullptr;
    QAction *m_undoAction = nullptr;
    QAction *m_redoAction = nullptr;
    QAction *m_createBoxAction = nullptr;
    QAction *m_createCylinderAction = nullptr;
    QAction *m_checkGmshAction = nullptr;
    QAction *m_generateMeshAction = nullptr;
    QAction *m_readMeshInfoAction = nullptr;
    QAction *m_showMeshAction = nullptr;
    QAction *m_pickFaceAction = nullptr;
    QAction *m_clearPickAction = nullptr;
    QAction *m_createFaceGroupFromPickAction = nullptr;
    QAction *m_addPickedFacesToFaceGroupAction = nullptr;
    QAction *m_removePickedFacesFromFaceGroupAction = nullptr;
    QAction *m_clearFaceGroupFacesAction = nullptr;
    QAction *m_renameFaceGroupAction = nullptr;
    QAction *m_deleteFaceGroupAction = nullptr;
    QAction *m_setFaceGroupLocalMeshSizeAction = nullptr;
    QAction *m_toggleFaceGroupPhysicalGroupAction = nullptr;
    QAction *m_exitAction = nullptr;
    QVector<QAction *> m_runSolverActions;

    QAction *m_createMaterialAction = nullptr;
    QAction *m_createBoundaryConditionAction = nullptr;
    QAction *m_createLoadAction = nullptr;
    QAction *m_editSolverDataAction = nullptr;
    QAction *m_deleteSolverDataAction = nullptr;
    QVector<QAction *> m_resultFieldActions;
    QVector<QAction *> m_resultScaleActions;
    QAction *m_exportScreenshotAction = nullptr;
    QAction *m_projectResourcesAction = nullptr;
    QAction *m_validateSamplesAction = nullptr;
    QAction *m_clearDiagnosticsAction = nullptr;

    ProjectTreePanel *m_projectTreePanel = nullptr;
    DiagnosticPanel *m_diagnosticPanel = nullptr;
    PropertyPanel *m_propertyPanel = nullptr;
    ResultPostprocessPanel *m_resultPostprocessPanel = nullptr;
    LogPanel *m_logPanel = nullptr;
    RenderView *m_renderView = nullptr;

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
