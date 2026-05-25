#pragma once

#include "solver/calculix/CalculiXCaseData.h"
#include "solver/calculix/CalculiXDeckTypes.h"

#include <QStringList>

#include <vector>

struct CalculiXBoundaryMapResult
{
    bool success = false;
    std::vector<CalculiXBoundaryExport> boundaries;
    QStringList errors;
};

class CalculiXBoundaryMapper
{
public:
    CalculiXBoundaryMapResult map(const CalculiXCaseData &caseData) const;
};
