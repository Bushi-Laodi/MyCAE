#include "ui/RecentProjectController.h"

#include "commands/UndoStackController.h"
#include "geometry/GeometryManager.h"
#include "project/ProjectModel.h"
#include "project/ProjectManager.h"
#include "ui/AppSettings.h"
#include "ui/MainWindowActions.h"
#include "ui/MainWindowDocks.h"
#include "workflow/ProjectWorkflowController.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QMenu>

RecentProjectController::RecentProjectController(RecentProjectContext context, RecentProjectCallbacks callbacks)
    : m_context(context)
    , m_callbacks(std::move(callbacks))
{
}

void RecentProjectController::openProject(const QString &projectFilePath) const
{
    if (projectFilePath.trimmed().isEmpty()) {
        return;
    }

    ProjectWorkflowController projectWorkflow(
        m_context.projectManager,
        m_context.geometryManager,
        m_context.projectModel,
        m_context.docks.projectTreePanel,
        m_context.docks.propertyPanel,
        m_context.docks.renderView,
        m_context.window
    );
    const ProjectWorkflowResult result = projectWorkflow.openProjectFile(projectFilePath);
    if (m_callbacks.writeLogMessages) {
        m_callbacks.writeLogMessages(result.logMessages);
    }
    if (result.success) {
        m_context.activeProjectFile = m_context.projectModel.project().projectFilePath;
        m_context.undoStackController.clear();
        m_context.appSettings.addRecentProject(m_context.activeProjectFile);
        updateActions();
    }
    if (m_callbacks.updateActionStates) {
        m_callbacks.updateActionStates();
    }
}

void RecentProjectController::updateActions() const
{
    const QStringList projects = m_context.appSettings.recentProjects();
    for (int i = 0; i < m_context.actions.recentProjects.size(); ++i) {
        QAction *action = m_context.actions.recentProjects.at(i);
        if (!action) {
            continue;
        }
        const bool visible = i < projects.size();
        action->setVisible(visible);
        if (!visible) {
            action->setData({});
            continue;
        }
        const QString path = projects.at(i);
        action->setText(QFileInfo(path).dir().dirName() + " - " + QDir::toNativeSeparators(path));
        action->setData(path);
    }
    if (m_context.actions.recentProjectsMenu) {
        m_context.actions.recentProjectsMenu->setEnabled(!projects.isEmpty());
    }
    if (m_context.actions.clearRecentProjects) {
        m_context.actions.clearRecentProjects->setEnabled(!projects.isEmpty());
    }
}

void RecentProjectController::clearProjects() const
{
    m_context.appSettings.clearRecentProjects();
    updateActions();
    if (m_callbacks.writeLog) {
        m_callbacks.writeLog("Recent projects cleared.");
    }
}
