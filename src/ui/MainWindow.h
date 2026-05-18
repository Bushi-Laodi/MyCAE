#pragma once

#include "geometry/BoxGeometry.h"
#include "geometry/GeometryManager.h"
#include "project/ProjectManager.h"

#include <QMainWindow>
#include <QVector>

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
    void createBox();
    void checkGmsh();
    void generateMesh();
    void readMeshInfo();
    void showMesh();
    void setCurrentProject(const Project &project);
    void loadProjectGeometries();
    void refreshGeometryTree();
    void showGeometryProperties(const QString &geometryName);
    void displayBoxGeometry(const BoxGeometry &box);
    void writeLog(const QString &message);

    QAction *m_newProjectAction = nullptr;
    QAction *m_openProjectAction = nullptr;
    QAction *m_createBoxAction = nullptr;
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
    Project m_currentProject;
    QVector<BoxGeometry> m_boxes;
    int m_selectedBoxIndex = -1;
};
