#pragma once

#include <QStringList>

class GeometryManager;
class ProjectManager;
class ProjectModel;
class ProjectTreePanel;
class PropertyPanel;
class QMainWindow;
class RenderView;
struct Project;

struct ProjectWorkflowResult
{
    bool success = false;
    bool canceled = false;
    QStringList logMessages;
};

class ProjectWorkflowController
{
public:
    ProjectWorkflowController(
        ProjectManager &projectManager,
        const GeometryManager &geometryManager,
        ProjectModel &projectModel,
        ProjectTreePanel *projectTreePanel,
        PropertyPanel *propertyPanel,
        RenderView *renderView,
        QMainWindow *window
    );

    ProjectWorkflowResult createProject() const;
    ProjectWorkflowResult openProject() const;
    ProjectWorkflowResult setCurrentProject(const Project &project) const;
    ProjectWorkflowResult loadGeometries() const;
    ProjectWorkflowResult loadMeshes() const;
    ProjectWorkflowResult loadSimulationCase() const;
    ProjectWorkflowResult saveSimulationCase() const;
    void refreshProjectTree() const;
    void refreshGeometryTree() const;
    void refreshMeshTree() const;
    void refreshFaceGroupTree() const;
    void refreshSolverDataTree() const;
    void refreshResultTree() const;

private:
    ProjectManager &m_projectManager;
    const GeometryManager &m_geometryManager;
    ProjectModel &m_projectModel;
    ProjectTreePanel *m_projectTreePanel = nullptr;
    PropertyPanel *m_propertyPanel = nullptr;
    RenderView *m_renderView = nullptr;
    QMainWindow *m_window = nullptr;
};
