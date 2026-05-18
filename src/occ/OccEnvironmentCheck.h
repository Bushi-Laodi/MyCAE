#pragma once

#include <QString>

struct OccEnvironmentCheckResult
{
    bool ok = false;
    QString message;
};

OccEnvironmentCheckResult runOccEnvironmentCheck();
