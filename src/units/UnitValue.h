#pragma once

#include "units/UnitSystem.h"

#include <QString>

struct UnitValue
{
    double value = 0.0;
    QString unit;
    UnitQuantity quantity = UnitQuantity::Unknown;
};
