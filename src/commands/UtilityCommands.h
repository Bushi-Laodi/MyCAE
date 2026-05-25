#pragma once

#include "commands/AppCommand.h"

#include <QString>

#include <memory>

class LogPanel;

std::unique_ptr<AppCommand> makeLogMessageCommand(LogPanel *logPanel, const QString &message);
