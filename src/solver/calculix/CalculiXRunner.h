#pragma once

#include "solver/plugin/SolverCaseContext.h"
#include "solver/plugin/SolverRunResult.h"

class CalculiXRunner
{
public:
    SolverRunResult run(const SolverCaseContext &context) const;
};
