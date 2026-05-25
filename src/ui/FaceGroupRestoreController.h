#pragma once

#include <QString>
#include <QStringList>

#include <functional>

class GeometryManager;
class ProjectManager;
class ProjectModel;
class QMainWindow;
struct MainWindowDockWidgets;
struct Selection;

struct FaceGroupRestoreCallbacks
{
    std::function<void(const QStringList &)> writeLogMessages;
    std::function<void(const Selection &)> applySelection;
    std::function<void()> updateActionStates;
};

struct FaceGroupRestoreContext
{
    ProjectManager &projectManager;
    GeometryManager &geometryManager;
    ProjectModel &projectModel;
    MainWindowDockWidgets &docks;
    QMainWindow *window = nullptr;
};

class FaceGroupRestoreController
{
public:
    FaceGroupRestoreController(FaceGroupRestoreContext context, FaceGroupRestoreCallbacks callbacks);

    void restoreAfterUndoStackChange(const QString &selectionId) const;

private:
    FaceGroupRestoreContext m_context;
    FaceGroupRestoreCallbacks m_callbacks;
};
