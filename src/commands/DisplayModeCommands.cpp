#include "commands/DisplayModeCommands.h"

#include "commands/CommandUtilities.h"
#include "ui/RenderView.h"

#include <QString>

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

class ToggleOrientationMarkerCommand final : public AppCommand
{
public:
    explicit ToggleOrientationMarkerCommand(WorkflowCommandContext context)
        : m_context(context)
    {
    }

    void execute() override
    {
        if (!m_context.renderView) {
            writeLogMessages(m_context, {"Display mode command failed: render view is not available."});
            return;
        }

        const bool enabled = !m_context.renderView->orientationMarkerVisible();
        m_context.renderView->setOrientationMarkerVisible(enabled);
        writeLogMessages(
            m_context,
            {enabled ? "XYZ orientation marker enabled." : "XYZ orientation marker disabled."}
        );
    }

private:
    WorkflowCommandContext m_context;
};

class ToggleMeshTransparencyCommand final : public AppCommand
{
public:
    explicit ToggleMeshTransparencyCommand(WorkflowCommandContext context)
        : m_context(context)
    {
    }

    void execute() override
    {
        if (!m_context.renderView) {
            writeLogMessages(m_context, {"Display mode command failed: render view is not available."});
            return;
        }

        const bool makeTransparent = m_context.renderView->primaryOpacity() >= 0.99;
        m_context.renderView->setMeshTransparent(makeTransparent);
        const int percent = makeTransparent ? 38 : 100;
        writeLogMessages(
            m_context,
            {QString::fromUtf8(u8"主模型不透明度已设置为 %1%。").arg(percent)}
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

std::unique_ptr<AppCommand> makeToggleMeshTransparencyCommand(WorkflowCommandContext context)
{
    return std::make_unique<ToggleMeshTransparencyCommand>(context);
}

std::unique_ptr<AppCommand> makeToggleOrientationMarkerCommand(WorkflowCommandContext context)
{
    return std::make_unique<ToggleOrientationMarkerCommand>(context);
}
