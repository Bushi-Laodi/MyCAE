#include "commands/UtilityCommands.h"

#include "ui/LogPanel.h"

#include <utility>

namespace
{
class LogMessageCommand final : public AppCommand
{
public:
    LogMessageCommand(LogPanel *logPanel, QString message)
        : m_logPanel(logPanel)
        , m_message(std::move(message))
    {
    }

    void execute() override
    {
        if (m_logPanel) {
            m_logPanel->appendMessage(m_message);
        }
    }

private:
    LogPanel *m_logPanel = nullptr;
    QString m_message;
};
}

std::unique_ptr<AppCommand> makeLogMessageCommand(LogPanel *logPanel, const QString &message)
{
    return std::make_unique<LogMessageCommand>(logPanel, message);
}
