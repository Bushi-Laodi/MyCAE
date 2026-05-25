#pragma once

#include "commands/AppCommand.h"

#include <QString>

#include <functional>
#include <map>
#include <memory>

class QAction;
class QObject;

class ActionRegistry
{
public:
    void registerCommand(const QString &commandId, std::unique_ptr<AppCommand> command);
    void bindAction(QAction *action, const QString &commandId, QObject *context);
    void registerActionCommand(
        const QString &commandId,
        QAction *action,
        QObject *context,
        std::unique_ptr<AppCommand> command
    );
    bool execute(const QString &commandId) const;
    void setAfterExecuteCallback(std::function<void()> callback);

private:
    std::map<QString, std::unique_ptr<AppCommand>> m_commands;
    std::function<void()> m_afterExecuteCallback;
};
