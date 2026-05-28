#pragma once

#include <QString>
#include <QStringList>

class ProjectModel;
class ProjectWorkflowController;
class SolverPluginManager;

struct SolverCaseWorkflowResult
{
    bool success = false;
    QString resultId;
    QStringList logMessages;
};

class SolverCaseWorkflowController
{
public:
    SolverCaseWorkflowController(
        ProjectModel &projectModel,
        const SolverPluginManager &solverPluginManager,
        ProjectWorkflowController &projectWorkflow
    );

    SolverCaseWorkflowResult runPlugin(const QString &pluginId) const;

private:
    ProjectModel &m_projectModel;
    const SolverPluginManager &m_solverPluginManager;
    ProjectWorkflowController &m_projectWorkflow;
};
