#include "commands/DisplayModeCommands.h"

#include "commands/CommandUtilities.h"
#include "ui/RenderView.h"

namespace
{
class ToggleGeometryEdgesCommand final : public AppCommand
{
public:
    explicit ToggleGeometryEdgesCommand(WorkflowCommandContext context)
        : m_context(context)
    {
    }

    void execute() override
    {
        if (!m_context.renderView) {
            writeLogMessages(m_context, {"Display mode command failed: render view is not available."});
            return;
        }

        const bool enabled = !m_context.renderView->geometryEdgesVisible();
        m_context.renderView->setGeometryEdgesVisible(enabled);
        writeLogMessages(
            m_context,
            {enabled ? "Geometry edge display enabled." : "Geometry edge display disabled."}
        );
    }

private:
    WorkflowCommandContext m_context;
};
}

std::unique_ptr<AppCommand> makeToggleGeometryEdgesCommand(WorkflowCommandContext context)
{
    return std::make_unique<ToggleGeometryEdgesCommand>(context);
}
