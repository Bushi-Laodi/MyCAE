#include "units/UnitSystem.h"

QString UnitSystem::internalUnit(UnitQuantity quantity)
{
    switch (quantity) {
    case UnitQuantity::Length:
        return "mm";
    case UnitQuantity::Stress:
        return "MPa";
    case UnitQuantity::Force:
        return "N";
    case UnitQuantity::Density:
        return "kg/m^3";
    case UnitQuantity::Acceleration:
        return "m/s^2";
    case UnitQuantity::Dimensionless:
        return {};
    case UnitQuantity::Unknown:
        return {};
    }
    return {};
}

QString UnitSystem::displayName(UnitQuantity quantity)
{
    switch (quantity) {
    case UnitQuantity::Length:
        return QString::fromUtf8(u8"长度");
    case UnitQuantity::Stress:
        return QString::fromUtf8(u8"应力/压力");
    case UnitQuantity::Force:
        return QString::fromUtf8(u8"力");
    case UnitQuantity::Density:
        return QString::fromUtf8(u8"密度");
    case UnitQuantity::Acceleration:
        return QString::fromUtf8(u8"加速度");
    case UnitQuantity::Dimensionless:
        return QString::fromUtf8(u8"无量纲");
    case UnitQuantity::Unknown:
        return QString::fromUtf8(u8"未知");
    }
    return QString::fromUtf8(u8"未知");
}
