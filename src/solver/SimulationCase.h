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
};

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
