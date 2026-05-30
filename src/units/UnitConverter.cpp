#include "units/UnitConverter.h"

#include <QHash>

namespace
{
QString normalizedUnit(QString unit)
{
    unit = unit.trimmed();
    unit.replace(QString::fromUtf8(u8"²"), "^2");
    unit.replace(QString::fromUtf8(u8"³"), "^3");
    return unit.toLower();
}

double factorFor(UnitQuantity quantity, const QString &unit)
{
    const QString normalized = normalizedUnit(unit);
    if (normalized.isEmpty()) {
        return 1.0;
    }

    if (quantity == UnitQuantity::Stress) {
        static const QHash<QString, double> factors{
            {"pa", 1.0e-6},
            {"kpa", 1.0e-3},
            {"mpa", 1.0},
            {"gpa", 1.0e3},
            {"n/mm^2", 1.0}
        };
        return factors.value(normalized, 1.0);
    }
    if (quantity == UnitQuantity::Force) {
        static const QHash<QString, double> factors{
            {"n", 1.0},
            {"kn", 1.0e3}
        };
        return factors.value(normalized, 1.0);
    }
    if (quantity == UnitQuantity::Density) {
        static const QHash<QString, double> factors{
            {"kg/m^3", 1.0},
            {"g/cm^3", 1.0e3},
            {"t/mm^3", 1.0e12}
        };
        return factors.value(normalized, 1.0);
    }
    if (quantity == UnitQuantity::Acceleration) {
        static const QHash<QString, double> factors{
            {"m/s^2", 1.0},
            {"mm/s^2", 1.0e-3}
        };
        return factors.value(normalized, 1.0);
    }
    if (quantity == UnitQuantity::Length) {
        static const QHash<QString, double> factors{
            {"mm", 1.0},
            {"m", 1.0e3},
            {"cm", 10.0}
        };
        return factors.value(normalized, 1.0);
    }
    return 1.0;
}
}

double UnitConverter::toInternal(UnitQuantity quantity, double value, const QString &unit)
{
    return value * factorFor(quantity, unit);
}

double UnitConverter::fromInternal(UnitQuantity quantity, double value, const QString &unit)
{
    return value / factorFor(quantity, unit);
}

double UnitConverter::stressToPa(double value, const QString &unit)
{
    return stressToMPa(value, unit) * 1.0e6;
}

double UnitConverter::lengthToMm(double value, const QString &unit)
{
    return toInternal(UnitQuantity::Length, value, unit);
}

double UnitConverter::lengthFromMm(double value, const QString &unit)
{
    return fromInternal(UnitQuantity::Length, value, unit);
}

double UnitConverter::stressToMPa(double value, const QString &unit)
{
    return toInternal(UnitQuantity::Stress, value, unit);
}

double UnitConverter::stressFromMPa(double value, const QString &unit)
{
    return fromInternal(UnitQuantity::Stress, value, unit);
}

double UnitConverter::pressureToMPa(double value, const QString &unit)
{
    return stressToMPa(value, unit);
}

double UnitConverter::forceToN(double value, const QString &unit)
{
    return toInternal(UnitQuantity::Force, value, unit);
}

double UnitConverter::forceFromN(double value, const QString &unit)
{
    return fromInternal(UnitQuantity::Force, value, unit);
}

double UnitConverter::densityToKgPerM3(double value, const QString &unit)
{
    return toInternal(UnitQuantity::Density, value, unit);
}

double UnitConverter::accelerationToMPerS2(double value, const QString &unit)
{
    return toInternal(UnitQuantity::Acceleration, value, unit);
}

bool UnitConverter::isKnownUnit(UnitQuantity quantity, const QString &unit)
{
    return knownUnits(quantity).contains(unit.trimmed(), Qt::CaseInsensitive);
}

QStringList UnitConverter::knownUnits(UnitQuantity quantity)
{
    switch (quantity) {
    case UnitQuantity::Length:
        return {"m", "mm", "cm"};
    case UnitQuantity::Stress:
        return {"Pa", "kPa", "MPa", "GPa", "N/mm^2"};
    case UnitQuantity::Force:
        return {"N", "kN"};
    case UnitQuantity::Density:
        return {"kg/m^3", "g/cm^3", "t/mm^3"};
    case UnitQuantity::Acceleration:
        return {"m/s^2", "mm/s^2"};
    case UnitQuantity::Dimensionless:
        return {""};
    case UnitQuantity::Unknown:
        return {};
    }
    return {};
}
