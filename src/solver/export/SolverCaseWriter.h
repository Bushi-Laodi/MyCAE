#pragma once

#include "solver/export/SolverCaseWriterResult.h"
#include "solver/plugin/SolverCaseContext.h"

class SolverPlugin;

class SolverCaseWriter
{
public:
    SolverCaseWriterResult writeCase(const SolverPlugin &plugin, const SolverCaseContext &context) const;
};
