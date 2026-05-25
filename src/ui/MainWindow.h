#pragma once

#include "geometry/GeometryManager.h"
#include "picking/PickController.h"
#include "project/ProjectModel.h"
#include "project/ProjectManager.h"
#include "project/SelectionState.h"
#include "result/ResultAnimationController.h"
#include "solver/plugin/SolverPluginManager.h"
#include "ui/ActionRegistry.h"

#include <QMainWindow>
#include <QVector>
#include <QStringList>

class QAction;
class QString;

class LogPanel;
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
    void handleFacePicked(const PickSelection &selection);
    void updateActionStates();
    WorkflowCommandContext workflowCommandContext();
    void writeLog(const QString &message);
    void writeLogMessages(const QStringList &messages);

    QAction *m_newProjectAction = nullptr;
    QAction *m_openProjectAction = nullptr;
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

    ProjectTreePanel *m_projectTreePanel = nullptr;
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
    ResultAnimationController m_resultAnimationController;
};
