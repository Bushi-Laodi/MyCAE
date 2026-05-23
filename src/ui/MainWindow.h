#pragma once

#include "geometry/GeometryCreationController.h"
#include "geometry/GeometryManager.h"
#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"
#include "project/ProjectModel.h"
#include "project/ProjectManager.h"
#include "solver/plugin/SolverPluginManager.h"

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
    void refreshGeometryTree();
    void refreshMeshTree();
    bool selectGeometryByName(const QString &geometryName);
    void showGeometryProperties(const QString &geometryName);
    void showMeshObject(const QString &meshName);
    void displayGeometry(const GeometryObject &geometry);
    void displayMeshObject(const MeshObject &meshObject);
    void writeLog(const QString &message);

    QAction *m_newProjectAction = nullptr;
    QAction *m_openProjectAction = nullptr;
    QAction *m_createBoxAction = nullptr;
    QAction *m_createCylinderAction = nullptr;
    QAction *m_checkGmshAction = nullptr;
    QAction *m_generateMeshAction = nullptr;
    QAction *m_readMeshInfoAction = nullptr;
    QAction *m_showMeshAction = nullptr;
    QAction *m_exitAction = nullptr;
    ProjectTreePanel *m_projectTreePanel = nullptr;
    PropertyPanel *m_propertyPanel = nullptr;
    LogPanel *m_logPanel = nullptr;
    RenderView *m_renderView = nullptr;

    ProjectManager m_projectManager;
    GeometryManager m_geometryManager;
    ProjectModel m_projectModel;
    SolverPluginManager m_solverPluginManager;
};
