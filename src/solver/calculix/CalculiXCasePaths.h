#pragma once

#include "solver/plugin/SolverCaseContext.h"

#include <QString>

struct CalculiXCasePaths
{
    QString caseDirectory;
    QString jobName;
    QString inputFile;
    QString datFile;
    QString staFile;
    QString frdFile;
    QString logFile;
};

class CalculiXCasePathsBuilder
{
public:
    static CalculiXCasePaths fromContext(const SolverCaseContext &context);
};
