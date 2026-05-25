#pragma once

#include "solver/calculix/CalculiXCaseData.h"
#include "solver/calculix/CalculiXDeckTypes.h"
#include "solver/calculix/CalculiXInputDeck.h"

#include <QStringList>

#include <vector>

class CalculiXLoadWriter
{
public:
    bool appendLoads(
        CalculiXInputDeck &deck,
        const CalculiXCaseData &caseData,
        const std::vector<CalculiXBoundaryExport> &boundaries,
        QStringList &errors
    ) const;
};
