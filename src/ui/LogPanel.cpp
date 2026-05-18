#include "LogPanel.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>

LogPanel::LogPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_logView = new QPlainTextEdit(this);
    m_logView->setReadOnly(true);
    layout->addWidget(m_logView);
}

void LogPanel::appendMessage(const QString &message)
{
    m_logView->appendPlainText(message);
}
