#pragma once

#include "solver/plugin/SolverCaseContext.h"
#include "solver/plugin/SolverResultReadResult.h"

class CalculiXResultReader
{
public:
    SolverResultReadResult read(const SolverCaseContext &context) const;
};
