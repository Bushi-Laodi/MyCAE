#pragma once

#include "solver/calculix/CalculiXCaseData.h"
#include "solver/plugin/SolverCaseContext.h"

class CalculiXCaseDataBuilder
{
public:
    CalculiXCaseDataBuildResult build(const SolverCaseContext &context) const;
};
