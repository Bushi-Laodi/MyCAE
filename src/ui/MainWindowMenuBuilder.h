#pragma once

class QMainWindow;

class ActionRegistry;
class LogPanel;
class SolverPluginManager;
struct MainWindowActions;
struct WorkflowCommandContext;

class MainWindowMenuBuilder
{
public:
    static void build(
        QMainWindow *window,
        MainWindowActions &actions,
        ActionRegistry &actionRegistry,
        const WorkflowCommandContext &context,
        const SolverPluginManager &solverPluginManager,
        LogPanel *logPanel
    );
};
