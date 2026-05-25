#pragma once

#include <QStringList>

struct SolverCapabilities
{
    bool canExportCase = false;
    bool canRunCase = false;
    bool canReadResult = false;
    QStringList analysisTypes;
    QStringList inputFormats;
    QStringList outputFormats;
};
