#pragma once

#include "units/UnitSystem.h"

#include <QString>
#include <QStringList>

class UnitConverter
{
public:
    static double toInternal(UnitQuantity quantity, double value, const QString &unit);
    static double fromInternal(UnitQuantity quantity, double value, const QString &unit);

    static double stressToPa(double value, const QString &unit);
    static double lengthToMm(double value, const QString &unit);
    static double lengthFromMm(double value, const QString &unit);
    static double stressToMPa(double value, const QString &unit);
    static double stressFromMPa(double value, const QString &unit);
    static double pressureToMPa(double value, const QString &unit);
    static double forceToN(double value, const QString &unit);
    static double forceFromN(double value, const QString &unit);
    static double densityToKgPerM3(double value, const QString &unit);
    static double accelerationToMPerS2(double value, const QString &unit);

    static bool isKnownUnit(UnitQuantity quantity, const QString &unit);
    static QStringList knownUnits(UnitQuantity quantity);
};
