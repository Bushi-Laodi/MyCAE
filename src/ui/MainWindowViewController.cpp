#include "ui/MainWindowViewController.h"

#include "diagnostics/DiagnosticCollector.h"
#include "project/ProjectModel.h"
#include "ui/DiagnosticPanel.h"
#include "ui/LogPanel.h"
#include "ui/MainWindowDocks.h"
#include "ui/ProjectTreePanel.h"
#include "ui/PropertyPanel.h"
#include "ui/ResultPostprocessPanel.h"

MainWindowViewController::MainWindowViewController(MainWindowViewContext context, MainWindowViewCallbacks callbacks)
    : m_context(context)
    , m_callbacks(std::move(callbacks))
{
}

void MainWindowViewController::refreshDiagnosticsPanel() const
{
    if (m_context.docks.diagnosticPanel) {
        m_context.docks.diagnosticPanel->setDiagnostics(m_context.diagnosticCollector.diagnostics());
    }
}

void MainWindowViewController::refreshResultViews() const
{
    if (m_context.docks.projectTreePanel) {
        m_context.docks.projectTreePanel->setResultItems(m_context.projectModel.resultRepository().results());
    }
    if (m_context.docks.propertyPanel) {
        m_context.docks.propertyPanel->showResultCategory(m_context.projectModel.resultRepository().results());
    }
    if (m_context.docks.resultPostprocessPanel) {
        m_context.docks.resultPostprocessPanel->setResult(m_context.projectModel.resultForSelection());
    }
    if (m_callbacks.updateActionStates) {
        m_callbacks.updateActionStates();
    }
}

void MainWindowViewController::writeLog(const QString &message) const
{
    if (m_context.docks.logPanel) {
        m_context.docks.logPanel->appendMessage(message);
    }
    if (m_context.diagnosticCollector.addFromLogMessage(message)) {
        refreshDiagnosticsPanel();
    }
}

void MainWindowViewController::writeLogMessages(const QStringList &messages) const
{
    for (const QString &message : messages) {
        writeLog(message);
    }
}
