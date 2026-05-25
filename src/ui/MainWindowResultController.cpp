#include "ui/MainWindowResultController.h"

#include "ui/MainWindowDocks.h"
#include "workflow/ResultWorkflowController.h"

MainWindowResultController::MainWindowResultController(
    MainWindowResultContext context,
    MainWindowResultCallbacks callbacks
)
    : m_context(context)
    , m_callbacks(std::move(callbacks))
{
}

void MainWindowResultController::setSelectedField(const QString &fieldName) const
{
    writeMessagesAndUpdate(workflow().setSelectedField(fieldName));
}

void MainWindowResultController::setSelectedDeformationScale(double scale) const
{
    writeMessagesAndUpdate(workflow().setSelectedDeformationScale(scale));
}

void MainWindowResultController::setSelectedMeshEdges(bool enabled) const
{
    writeMessagesAndUpdate(workflow().setSelectedMeshEdges(enabled));
}

void MainWindowResultController::setSelectedUndeformedOverlay(bool enabled) const
{
    writeMessagesAndUpdate(workflow().setSelectedUndeformedOverlay(enabled));
}

void MainWindowResultController::playSelectedAnimation(double speed) const
{
    writeMessagesAndUpdate(workflow().playSelectedAnimation(speed));
}

void MainWindowResultController::stopSelectedAnimation() const
{
    writeMessagesAndUpdate(workflow().stopSelectedAnimation());
}

void MainWindowResultController::applyAnimatedDeformationScale(double scale) const
{
    writeMessages(workflow().applyAnimatedDeformationScale(scale));
}

void MainWindowResultController::exportSelectedCsv() const
{
    writeMessages(workflow().exportSelectedCsv());
}

void MainWindowResultController::exportSelectedReport() const
{
    writeMessages(workflow().exportSelectedReport());
}

void MainWindowResultController::exportRenderScreenshot() const
{
    writeMessages(workflow().exportRenderScreenshot());
}

void MainWindowResultController::openSelectedResultDirectory() const
{
    writeMessages(workflow().openSelectedResultDirectory());
}

void MainWindowResultController::renameSelectedResult() const
{
    writeMessagesAndUpdate(workflow().renameSelectedResult());
}

void MainWindowResultController::deleteSelectedResultHistory() const
{
    writeMessagesAndUpdate(workflow().deleteSelectedResultHistory());
}

ResultWorkflowController MainWindowResultController::workflow() const
{
    return ResultWorkflowController(
        m_context.projectModel,
        m_context.docks.projectTreePanel,
        m_context.docks.propertyPanel,
        m_context.docks.resultPostprocessPanel,
        m_context.docks.renderView,
        m_context.appSettings,
        m_context.resultAnimationController,
        m_context.parent
    );
}

void MainWindowResultController::writeMessages(const QStringList &messages) const
{
    if (m_callbacks.writeLogMessages) {
        m_callbacks.writeLogMessages(messages);
    }
}

void MainWindowResultController::writeMessagesAndUpdate(const QStringList &messages) const
{
    writeMessages(messages);
    if (m_callbacks.updateActionStates) {
        m_callbacks.updateActionStates();
    }
}
