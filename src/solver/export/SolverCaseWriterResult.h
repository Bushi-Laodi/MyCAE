#pragma once

#include <QString>
#include <QStringList>

struct SolverCaseWriterResult
{
    bool success = false;
    QString caseRootPath;
    QStringList writtenFiles;
    QStringList warnings;
    QStringList errors;
    QStringList logMessages;
};
