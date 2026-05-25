#pragma once

#include <QString>

#include <functional>

class ActionRegistry;
class AppSettings;
class QMainWindow;
class ProjectModel;
class ResultAnimationController;
class ResultPostprocessPanel;
class UndoStackController;

struct MainWindowLifecycleCallbacks
{
    std::function<void(const QString &)> faceGroupRestored;
    std::function<void(double)> resultAnimationFrameScaleChanged;
    std::function<void()> updateRecentProjects;
    std::function<void()> updateActionStates;
};

struct MainWindowLifecycleContext
{
    QMainWindow &window;
    ActionRegistry &actionRegistry;
    UndoStackController &undoStackController;
    AppSettings &appSettings;
    ProjectModel &projectModel;
    ResultAnimationController &resultAnimationController;
    ResultPostprocessPanel *resultPostprocessPanel = nullptr;
    QString &activeProjectFile;
};

class MainWindowLifecycleController
{
public:
    static void install(const MainWindowLifecycleContext &context, const MainWindowLifecycleCallbacks &callbacks);
    static void saveWindowState(AppSettings &appSettings, QMainWindow &window);
};
