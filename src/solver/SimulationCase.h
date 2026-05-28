#pragma once

#include "geometry/FaceGroup.h"
#include "mesh/MeshSetup.h"
#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "solver/Material.h"

#include <QString>

#include <vector>

enum class FlowSolverType
{
    Simple
};

enum class TurbulenceModel
{
    KOmegaSST
};

enum class AxisDirection
{
    X,
    Y,
    Z
};

enum class BooleanOperationType
{
    Union
};

enum class AnalysisDomain
{
    Structural,
    Cfd
};

struct Point3D
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct CylinderDefinition
{
    QString id;
    QString name;
    Point3D origin;
    AxisDirection direction = AxisDirection::Z;
    double length = 0.0;
    double radius = 0.0;
};

struct BooleanOperationDefinition
{
    QString id;
    BooleanOperationType type = BooleanOperationType::Union;
    std::vector<QString> inputGeometryIds;
    QString resultGeometryName;
};

struct GeometrySetup
{
    std::vector<CylinderDefinition> cylinders;
    BooleanOperationDefinition booleanOperation;
    std::vector<FaceGroup> faceGroups;
};

struct RunControl
{
    double endTime = 400.0;
    double timeStep = 1.0;
    double writeInterval = 100.0;
    bool cleanPreviousResult = false;
};

struct StructuralCase
{
    QString id;
    QString name;
    QString sourceGeometryName;
    QString meshName;
    std::vector<Material> materials;
    std::vector<BoundaryCondition> constraints;
    std::vector<Load> loads;
};

struct CfdCase
{
    QString id;
    QString name;
    QString sourceGeometryName;
    QString meshName;
    FlowSolverType solverType = FlowSolverType::Simple;
    TurbulenceModel turbulenceModel = TurbulenceModel::KOmegaSST;
    RunControl runControl;
    std::vector<Material> materials;
    std::vector<BoundaryCondition> boundaries;
    std::vector<Load> fieldValues;
};

struct SimulationCase
{
    QString id;
    QString name;
    QString sourceGeometryName;
    QString meshName;
    GeometrySetup geometrySetup;
    MeshSetup meshSetup;
    FlowSolverType solverType = FlowSolverType::Simple;
    TurbulenceModel turbulenceModel = TurbulenceModel::KOmegaSST;
    RunControl runControl;
    QString postProcessingTool;
    std::vector<Material> materials;
    std::vector<BoundaryCondition> boundaryConditions;
    std::vector<Load> loads;
    StructuralCase structuralCase;
    CfdCase cfdCase;
};

inline bool isStructuralMaterial(const Material &material)
{
    return material.domain == MaterialDomain::Solid;
}

inline bool isCfdMaterial(const Material &material)
{
    return material.domain == MaterialDomain::Fluid;
}

inline bool isCfdBoundaryType(BoundaryConditionType type)
{
    return type == BoundaryConditionType::Wall
        || type == BoundaryConditionType::VelocityInlet
        || type == BoundaryConditionType::PressureInlet
        || type == BoundaryConditionType::PressureOutlet
        || type == BoundaryConditionType::Symmetry;
}

inline bool isStructuralLoadType(LoadType type)
{
    return type == LoadType::Pressure || type == LoadType::BodyForce;
}

inline bool isCfdFieldValueType(LoadType type)
{
    return type == LoadType::Velocity || type == LoadType::Pressure;
}

inline bool hasLoadForBoundary(const std::vector<Load> &loads, const QString &boundaryConditionId)
{
    for (const Load &load : loads) {
        if (load.enabled && load.boundaryConditionId == boundaryConditionId) {
            return true;
        }
    }
    return false;
}

inline bool hasStructuralLoadForBoundary(const std::vector<Load> &loads, const QString &boundaryConditionId)
{
    for (const Load &load : loads) {
        if (load.enabled
                && load.boundaryConditionId == boundaryConditionId
                && isStructuralLoadType(load.type)) {
            return true;
        }
    }
    return false;
}

inline bool hasCfdFieldValueForBoundary(const std::vector<Load> &loads, const QString &boundaryConditionId)
{
    for (const Load &load : loads) {
        if (load.enabled
                && load.boundaryConditionId == boundaryConditionId
                && isCfdFieldValueType(load.type)) {
            return true;
        }
    }
    return false;
}

inline bool isStructuralConstraint(const BoundaryCondition &boundaryCondition, const std::vector<Load> &loads)
{
    if (!boundaryCondition.enabled) {
        return false;
    }
    if (hasStructuralLoadForBoundary(loads, boundaryCondition.id)) {
        return true;
    }
    return boundaryCondition.type == BoundaryConditionType::Wall
        && !hasCfdFieldValueForBoundary(loads, boundaryCondition.id);
}

inline bool isCfdBoundary(const BoundaryCondition &boundaryCondition)
{
    return boundaryCondition.enabled && isCfdBoundaryType(boundaryCondition.type);
}

inline QString toString(AnalysisDomain domain)
{
    switch (domain) {
    case AnalysisDomain::Structural:
        return "structural";
    case AnalysisDomain::Cfd:
        return "cfd";
    }
    return "structural";
}

inline QString toString(FlowSolverType solverType)
{
    switch (solverType) {
    case FlowSolverType::Simple:
        return "SIMPLE";
    }
    return "SIMPLE";
}

inline QString toString(TurbulenceModel model)
{
    switch (model) {
    case TurbulenceModel::KOmegaSST:
        return "k-Omega SST";
    }
    return "k-Omega SST";
}

inline QString toString(AxisDirection direction)
{
    switch (direction) {
    case AxisDirection::X:
        return "X";
    case AxisDirection::Y:
        return "Y";
    case AxisDirection::Z:
        return "Z";
    }
    return "Z";
}

inline QString toString(BooleanOperationType type)
{
    switch (type) {
    case BooleanOperationType::Union:
        return "union";
    }
    return "union";
}
