#pragma once

#include "solver/export/SolverCaseWriterResult.h"
#include "solver/plugin/SolverCaseContext.h"

class CalculiXCaseWriter
{
public:
    SolverCaseWriterResult write(const SolverCaseContext &context) const;
};
