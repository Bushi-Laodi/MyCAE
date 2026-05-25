#pragma once

#include <QString>
#include <QStringList>

struct SolverRunResult
{
    bool success = false;
    int exitCode = -1;
    QString command;
    QString workingDirectory;
    QString standardOutput;
    QString standardError;
    QString logFile;
    QStringList errors;
    QStringList logMessages;
};
