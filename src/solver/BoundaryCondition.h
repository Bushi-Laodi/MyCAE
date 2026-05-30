#pragma once

#include <QString>

enum class BoundaryTargetKind
{
    GeometryFaceGroup,
    MeshBoundary
};

enum class BoundaryConditionType
{
    FixedSupport,
    Displacement,
    LoadTarget,
    Wall,
    VelocityInlet,
    PressureInlet,
    PressureOutlet,
    Symmetry,
    SymmetryStructural,
    Unknown
};

struct DisplacementConstraint
{
    bool uxEnabled = true;
    bool uyEnabled = true;
    bool uzEnabled = true;
    double ux = 0.0;
    double uy = 0.0;
    double uz = 0.0;
    QString unit;
};

struct BoundaryTarget
{
    BoundaryTargetKind kind = BoundaryTargetKind::GeometryFaceGroup;
    QString geometryName;
    QString faceGroupId;
    QString faceGroupName;
    QString meshBoundaryName;
};

struct BoundaryCondition
{
    QString id;
    QString name;
    BoundaryConditionType type = BoundaryConditionType::Unknown;
    BoundaryTarget target;
    DisplacementConstraint displacement;
    QString symmetryNormal = "Z";
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
    case BoundaryConditionType::FixedSupport:
        return "fixedSupport";
    case BoundaryConditionType::Displacement:
        return "displacement";
    case BoundaryConditionType::LoadTarget:
        return "loadTarget";
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
    case BoundaryConditionType::SymmetryStructural:
        return "symmetryStructural";
    case BoundaryConditionType::Unknown:
        return "unknown";
    }
    return "unknown";
}
