#pragma once

#include <QString>

enum class BoundaryTargetKind
{
    GeometryFaceGroup,
    MeshBoundary
};

enum class BoundaryConditionType
{
    Wall,
    VelocityInlet,
    PressureInlet,
    PressureOutlet,
    Symmetry,
    Unknown
};

struct BoundaryTarget
{
    BoundaryTargetKind kind = BoundaryTargetKind::GeometryFaceGroup;
    QString geometryName;
    QString faceGroupName;
    QString meshBoundaryName;
};

struct BoundaryCondition
{
    QString id;
    QString name;
    BoundaryConditionType type = BoundaryConditionType::Unknown;
    BoundaryTarget target;
    QString materialId;
    bool enabled = true;
};

inline QString toString(BoundaryTargetKind kind)
{
    switch (kind) {
    case BoundaryTargetKind::GeometryFaceGroup:
        return "geometryFaceGroup";
    case BoundaryTargetKind::MeshBoundary:
        return "meshBoundary";
    }
    return "geometryFaceGroup";
}

inline QString toString(BoundaryConditionType type)
{
    switch (type) {
    case BoundaryConditionType::Wall:
        return "wall";
    case BoundaryConditionType::VelocityInlet:
        return "velocityInlet";
    case BoundaryConditionType::PressureInlet:
        return "pressureInlet";
    case BoundaryConditionType::PressureOutlet:
        return "pressureOutlet";
    case BoundaryConditionType::Symmetry:
        return "symmetry";
    case BoundaryConditionType::Unknown:
        return "unknown";
    }
    return "unknown";
}
