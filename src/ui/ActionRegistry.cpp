#include "ui/ActionRegistry.h"

#include "commands/AppCommand.h"

#include <QAction>

#include <utility>

void ActionRegistry::registerCommand(const QString &commandId, std::unique_ptr<AppCommand> command)
{
    if (!command) {
        return;
    }

    m_commands.insert_or_assign(commandId, std::move(command));
}

void ActionRegistry::bindAction(QAction *action, const QString &commandId, QObject *context)
{
    if (!action) {
        return;
    }

    action->setData(commandId);
    QObject *connectionContext = context ? context : action;
    QObject::connect(action, &QAction::triggered, connectionContext, [this, action]() {
        execute(action->data().toString());
    });
}

void ActionRegistry::registerActionCommand(
    const QString &commandId,
    QAction *action,
    QObject *context,
    std::unique_ptr<AppCommand> command
)
{
    registerCommand(commandId, std::move(command));
    bindAction(action, commandId, context);
}

bool ActionRegistry::execute(const QString &commandId) const
{
    const auto it = m_commands.find(commandId);
    if (it == m_commands.end() || !it->second) {
        return false;
    }

    it->second->execute();
    if (m_afterExecuteCallback) {
        m_afterExecuteCallback();
    }
    return true;
}

void ActionRegistry::setAfterExecuteCallback(std::function<void()> callback)
{
    m_afterExecuteCallback = std::move(callback);
}
