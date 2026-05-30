#pragma once

#include <QString>

enum class LoadType
{
    Velocity,
    Force,
    SurfaceForce,
    Traction,
    Pressure,
    Gravity,
    BodyForce,
    Temperature,
    Unknown
};

enum class LoadValueKind
{
    Scalar,
    Vector3
};

struct LoadValue
{
    LoadValueKind kind = LoadValueKind::Scalar;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    QString unit;
};

struct Load
{
    QString id;
    QString name;
    LoadType type = LoadType::Unknown;
    QString boundaryConditionId;
    QString fieldName;
    LoadValue value;
    bool enabled = true;
};

inline QString toString(LoadType type)
{
    switch (type) {
    case LoadType::Velocity:
        return "velocity";
    case LoadType::Force:
        return "force";
    case LoadType::SurfaceForce:
        return "surfaceForce";
    case LoadType::Traction:
        return "traction";
    case LoadType::Pressure:
        return "pressure";
    case LoadType::Gravity:
        return "gravity";
    case LoadType::BodyForce:
        return "bodyForce";
    case LoadType::Temperature:
        return "temperature";
    case LoadType::Unknown:
        return "unknown";
    }
    return "unknown";
}

inline QString toString(LoadValueKind kind)
{
    switch (kind) {
    case LoadValueKind::Scalar:
        return "scalar";
    case LoadValueKind::Vector3:
        return "vector3";
    }
    return "scalar";
}
