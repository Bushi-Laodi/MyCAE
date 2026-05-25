#include "commands/PickModeCommands.h"

#include "commands/CommandUtilities.h"
#include "picking/PickController.h"

namespace
{
class ToggleFacePickModeCommand final : public AppCommand
{
public:
    explicit ToggleFacePickModeCommand(WorkflowCommandContext context)
        : m_context(context)
    {
    }

    void execute() override
    {
        if (!m_context.pickController) {
            writeLogMessages(m_context.logPanel, {"Pick command failed: pick controller is not available."});
            return;
        }

        if (m_context.pickController->mode() == PickMode::Face) {
            writeLogMessages(
                m_context.logPanel,
                m_context.pickController->setMode(PickMode::None, m_context.renderView).logMessages
            );
            writeLogMessages(m_context.logPanel, m_context.pickController->clear(m_context.renderView).logMessages);
            return;
        }

        writeLogMessages(
            m_context.logPanel,
            m_context.pickController->setMode(PickMode::Face, m_context.renderView).logMessages
        );
    }

private:
    WorkflowCommandContext m_context;
};

class ClearPickCommand final : public AppCommand
{
public:
    explicit ClearPickCommand(WorkflowCommandContext context)
        : m_context(context)
    {
    }

    void execute() override
    {
        if (!m_context.pickController) {
            writeLogMessages(m_context.logPanel, {"Clear pick failed: pick controller is not available."});
            return;
        }
        writeLogMessages(m_context.logPanel, m_context.pickController->clear(m_context.renderView).logMessages);
    }

private:
    WorkflowCommandContext m_context;
};
}

std::unique_ptr<AppCommand> makePickModeCommand(WorkflowCommandContext context)
{
    return std::make_unique<ToggleFacePickModeCommand>(context);
}

std::unique_ptr<AppCommand> makeClearPickCommand(WorkflowCommandContext context)
{
    return std::make_unique<ClearPickCommand>(context);
}
