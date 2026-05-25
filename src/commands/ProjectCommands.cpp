#include "commands/ProjectCommands.h"

#include "commands/CommandUtilities.h"
#include "picking/PickController.h"
#include "workflow/ProjectWorkflowController.h"

#include <QMainWindow>

namespace
{
class ProjectCommand final : public AppCommand
{
public:
    ProjectCommand(WorkflowCommandContext context, ProjectCommandType type)
        : m_context(context)
        , m_type(type)
    {
    }

    void execute() override
    {
        ProjectWorkflowController projectWorkflow = makeProjectWorkflow(m_context);
        const ProjectWorkflowResult result = m_type == ProjectCommandType::Create
            ? projectWorkflow.createProject()
            : projectWorkflow.openProject();
        if (result.success && m_context.pickController) {
            m_context.pickController->setMode(PickMode::None, m_context.renderView);
            m_context.pickController->clear(m_context.renderView);
        }
        writeLogMessages(m_context.logPanel, result.logMessages);
    }

private:
    WorkflowCommandContext m_context;
    ProjectCommandType m_type;
};

class CloseWindowCommand final : public AppCommand
{
public:
    explicit CloseWindowCommand(QMainWindow *window)
        : m_window(window)
    {
    }

    void execute() override
    {
        if (m_window) {
            m_window->close();
        }
    }

private:
    QMainWindow *m_window = nullptr;
};
}

std::unique_ptr<AppCommand> makeProjectCommand(WorkflowCommandContext context, ProjectCommandType type)
{
    return std::make_unique<ProjectCommand>(context, type);
}

std::unique_ptr<AppCommand> makeCloseWindowCommand(QMainWindow *window)
{
    return std::make_unique<CloseWindowCommand>(window);
}
