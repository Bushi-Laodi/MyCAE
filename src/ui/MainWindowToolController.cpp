#include "ui/MainWindowToolController.h"

#include "diagnostics/DiagnosticCollector.h"
#include "project/ProjectModel.h"
#include "ui/MainWindowDocks.h"
#include "ui/ProjectResourceDialog.h"
#include "ui/SampleValidationDialog.h"

#include <QTimer>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}
}

MainWindowToolController::MainWindowToolController(MainWindowToolContext context, MainWindowToolCallbacks callbacks)
    : m_context(context)
    , m_callbacks(std::move(callbacks))
{
}

void MainWindowToolController::showProjectResources() const
{
    if (!m_context.projectModel.hasProject()) {
        if (m_callbacks.writeLog) {
            m_callbacks.writeLog(zh(u8"跳过工程资源检查：请先打开工程。"));
        }
        return;
    }

    ProjectResourceDialog dialog(m_context.projectModel, m_context.parent);
    QObject::connect(&dialog, &ProjectResourceDialog::logMessagesReady, m_context.parent, [this](const QStringList &messages) {
        if (m_callbacks.writeLogMessages) {
            m_callbacks.writeLogMessages(messages);
        }
    });
    QObject::connect(&dialog, &ProjectResourceDialog::resultsChanged, m_context.parent, [this]() {
        if (m_callbacks.refreshResultViews) {
            m_callbacks.refreshResultViews();
        }
    });
    dialog.exec();
}

void MainWindowToolController::validateSamples() const
{
    SampleValidationDialog dialog(m_context.parent);
    QObject::connect(&dialog, &SampleValidationDialog::logMessagesReady, m_context.parent, [this](const QStringList &messages) {
        if (m_callbacks.writeLogMessages) {
            m_callbacks.writeLogMessages(messages);
        }
    });
    QTimer::singleShot(0, &dialog, &SampleValidationDialog::runValidation);
    dialog.exec();
}

void MainWindowToolController::clearDiagnostics() const
{
    m_context.diagnosticCollector.clear();
    if (m_callbacks.refreshDiagnosticsPanel) {
        m_callbacks.refreshDiagnosticsPanel();
    }
    if (m_callbacks.writeLog) {
        m_callbacks.writeLog(zh(u8"诊断信息已清空。"));
    }
}
