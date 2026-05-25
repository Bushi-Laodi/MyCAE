#pragma once

#include <QString>
#include <QStringList>

struct SolverResultReadResult
{
    bool success = false;
    QString summary;
    QStringList resultFiles;
    QStringList warnings;
    QStringList errors;
    QStringList logMessages;
};
