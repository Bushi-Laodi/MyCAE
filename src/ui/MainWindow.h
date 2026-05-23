#pragma once

#include "geometry/GeometryCreationController.h"
#include "geometry/GeometryManager.h"
#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"
#include "mesh/MeshWorkflowController.h"
#include "project/ProjectModel.h"
#include "project/ProjectManager.h"
#include "solver/plugin/SolverPluginManager.h"
#include "ui/SolverDataController.h"

#include <QMainWindow>

class QAction;
class QString;

class LogPanel;
class ProjectTreePanel;
class PropertyPanel;
class RenderView;

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

    void newProject();
    void openProject();
    void createGeometry(GeometryCreateType type);
    void checkGmsh();
    void generateMesh();
    void readMeshInfo();
    void showMesh();
    void runSolverPlugin(const QString &pluginId);
    void setCurrentProject(const Project &project);
    void loadProjectGeometries();
    void loadProjectMeshes();
    void loadProjectSimulationCase();
    bool saveProjectSimulationCase();
    void refreshGeometryTree();
    void refreshMeshTree();
    void refreshSolverDataTree();
    bool selectGeometryByName(const QString &geometryName);
    void showGeometryProperties(const QString &geometryName);
    void showMeshObject(const QString &meshName);
    void showMaterial(const QString &materialId);
    void showBoundaryCondition(const QString &boundaryConditionId);
    void showLoad(const QString &loadId);
    void displayGeometry(const GeometryObject &geometry);
    void displayMeshObject(const MeshObject &meshObject);
    void handleMeshWorkflowResult(const MeshWorkflowResult &result);
    void handleSolverDataResult(const SolverDataControllerResult &result);
    void writeLog(const QString &message);

    // Solver data UI handlers
    void onMaterialCategorySelected();
    void onBoundaryConditionCategorySelected();
    void onLoadCategorySelected();
    void onSolverCategorySelected();
    void createMaterial();
    void createBoundaryCondition();
    void createLoad();
    void editSelectedSolverData();
    void deleteSelectedSolverData();

    QAction *m_newProjectAction = nullptr;
    QAction *m_openProjectAction = nullptr;
    QAction *m_createBoxAction = nullptr;
    QAction *m_createCylinderAction = nullptr;
    QAction *m_checkGmshAction = nullptr;
    QAction *m_generateMeshAction = nullptr;
    QAction *m_readMeshInfoAction = nullptr;
    QAction *m_showMeshAction = nullptr;
    QAction *m_exitAction = nullptr;

    // Solver data actions
    QAction *m_createMaterialAction = nullptr;
    QAction *m_createBoundaryConditionAction = nullptr;
    QAction *m_createLoadAction = nullptr;
    QAction *m_editSolverDataAction = nullptr;
    QAction *m_deleteSolverDataAction = nullptr;

    ProjectTreePanel *m_projectTreePanel = nullptr;
    PropertyPanel *m_propertyPanel = nullptr;
    LogPanel *m_logPanel = nullptr;
    RenderView *m_renderView = nullptr;

    ProjectManager m_projectManager;
    GeometryManager m_geometryManager;
    ProjectModel m_projectModel;
    SolverPluginManager m_solverPluginManager;
};
