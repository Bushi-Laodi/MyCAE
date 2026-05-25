#pragma once

#include "result/ResultObject.h"

#include <QString>
#include <QStringList>

struct CalculiXResultReadResult
{
    bool success = false;
    QString summary;
    QStringList resultFiles;
    QStringList warnings;
    QStringList errors;
    QStringList logMessages;
    ResultObject resultObject;
};

class CalculiXResultReader
{
public:
    CalculiXResultReadResult read(const QString &caseDirectory, const QString &jobName = "job") const;
};
