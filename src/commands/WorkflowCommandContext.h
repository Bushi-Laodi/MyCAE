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
class SolverPreflightPanel;
class SolverPluginManager;
class UndoStackController;

#include <functional>
#include <QStringList>

struct WorkflowCommandContext
{
    ProjectManager &projectManager;
    GeometryManager &geometryManager;
    ProjectModel &projectModel;
    const SolverPluginManager &solverPluginManager;
    ProjectTreePanel *projectTreePanel = nullptr;
    PropertyPanel *propertyPanel = nullptr;
    RenderView *renderView = nullptr;
    SolverPreflightPanel *solverPreflightPanel = nullptr;
    LogPanel *logPanel = nullptr;
    QMainWindow *window = nullptr;
    PickController *pickController = nullptr;
    UndoStackController *undoStackController = nullptr;
    std::function<void(const QStringList &)> writeLogMessages;
};
