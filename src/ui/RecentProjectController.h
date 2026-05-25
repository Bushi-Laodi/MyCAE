#pragma once

#include <QString>
#include <QStringList>

#include <functional>

class AppSettings;
class GeometryManager;
class ProjectManager;
class ProjectModel;
class QMainWindow;
class UndoStackController;
struct MainWindowActions;
struct MainWindowDockWidgets;

struct RecentProjectCallbacks
{
    std::function<void(const QString &)> writeLog;
    std::function<void(const QStringList &)> writeLogMessages;
    std::function<void()> updateActionStates;
};

struct RecentProjectContext
{
    ProjectManager &projectManager;
    GeometryManager &geometryManager;
    ProjectModel &projectModel;
    MainWindowActions &actions;
    MainWindowDockWidgets &docks;
    UndoStackController &undoStackController;
    AppSettings &appSettings;
    QString &activeProjectFile;
    QMainWindow *window = nullptr;
};

class RecentProjectController
{
public:
    RecentProjectController(RecentProjectContext context, RecentProjectCallbacks callbacks);

    void openProject(const QString &projectFilePath) const;
    void updateActions() const;
    void clearProjects() const;

private:
    RecentProjectContext m_context;
    RecentProjectCallbacks m_callbacks;
};
