#include "LogPanel.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>

LogPanel::LogPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);

    m_logView = new QPlainTextEdit(this);
    m_logView->setObjectName("log.view");
    m_logView->setReadOnly(true);
    m_logView->setPlaceholderText("No log messages yet.");

    layout->addWidget(m_logView);
}

void LogPanel::appendMessage(const QString &message)
{
    m_logView->appendPlainText(message);
}
