#pragma once

class GeometryManager;
class LogPanel;
class PickController;
class ProjectManager;
class ProjectModel;
class ProjectTreePanel;
class PropertyPanel;
class QMainWindow;
class RenderView;
class SolverPluginManager;

struct WorkflowCommandContext
{
    ProjectManager &projectManager;
    GeometryManager &geometryManager;
    ProjectModel &projectModel;
    const SolverPluginManager &solverPluginManager;
    ProjectTreePanel *projectTreePanel = nullptr;
    PropertyPanel *propertyPanel = nullptr;
    RenderView *renderView = nullptr;
    LogPanel *logPanel = nullptr;
    QMainWindow *window = nullptr;
    PickController *pickController = nullptr;
};
