#pragma once

#include "solver/plugin/SolverPlugin.h"

class OpenFoamPlugin final : public SolverPlugin
{
public:
    SolverPluginDescriptor descriptor() const override;
    SolverCaseWriterResult exportCase(const SolverCaseContext &context) const override;
    SolverRunResult runCase(const SolverCaseContext &context) const override;
    SolverResultReadResult readResult(const SolverCaseContext &context) const override;
};
