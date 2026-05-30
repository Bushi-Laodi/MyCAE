#pragma once

#include <QString>

enum class UnitQuantity
{
    Length,
    Stress,
    Force,
    Density,
    Acceleration,
    Dimensionless,
    Unknown
};

class UnitSystem
{
public:
    static QString internalUnit(UnitQuantity quantity);
    static QString displayName(UnitQuantity quantity);
};
