#pragma once

#include "solver/plugin/SolverCaseContext.h"
#include "solver/plugin/SolverResultReadResult.h"
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
    SolverResultReadResult read(const SolverCaseContext &context) const;
    CalculiXResultReadResult read(const QString &caseDirectory, const QString &jobName = "job") const;
};
