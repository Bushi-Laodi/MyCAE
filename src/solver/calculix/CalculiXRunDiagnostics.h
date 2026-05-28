#pragma once

#include "solver/calculix/CalculiXCasePaths.h"
#include "solver/plugin/SolverRunResult.h"

#include <QStringList>

struct CalculiXRunDiagnosticReport
{
    QStringList errors;
    QStringList warnings;
    QStringList hints;

    bool hasFindings() const;
};

class CalculiXRunDiagnostics
{
public:
    CalculiXRunDiagnosticReport analyze(
        const CalculiXCasePaths &paths,
        const SolverRunResult &runResult
    ) const;
};
