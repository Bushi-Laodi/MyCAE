#pragma once

#include "solver/calculix/CalculiXCaseData.h"
#include "solver/calculix/CalculiXInputDeck.h"

#include <QStringList>

struct CalculiXInputDeckBuildResult
{
    bool success = false;
    CalculiXInputDeck deck;
    QStringList warnings;
    QStringList errors;
    QStringList logMessages;
};

class CalculiXInputDeckBuilder
{
public:
    CalculiXInputDeckBuildResult build(const CalculiXCaseData &caseData) const;
};
