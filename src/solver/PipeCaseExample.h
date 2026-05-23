#pragma once

#include "solver/SimulationCase.h"

inline SimulationCase createPipeSimulationCaseExample()
{
    SimulationCase simulationCase;
    simulationCase.id = "pipe_case";
    simulationCase.name = "Pipe";
    simulationCase.sourceGeometryName = "Pipe";
    simulationCase.meshName = "Pipe_Mesh";
    simulationCase.geometrySetup.cylinders.push_back(CylinderDefinition{
        "cylinder_1",
        "Cylinder1",
        Point3D{0.0, 0.0, 0.0},
        AxisDirection::Z,
        0.4,
        0.05
    });
    simulationCase.geometrySetup.cylinders.push_back(CylinderDefinition{
        "cylinder_2",
        "Cylinder2",
        Point3D{0.0, -0.4, 0.0},
        AxisDirection::Y,
        1.5,
        0.075
    });
    simulationCase.geometrySetup.booleanOperation.id = "pipe_union";
    simulationCase.geometrySetup.booleanOperation.type = BooleanOperationType::Union;
    simulationCase.geometrySetup.booleanOperation.inputGeometryIds = {"cylinder_1", "cylinder_2"};
    simulationCase.geometrySetup.booleanOperation.resultGeometryName = "Pipe";
    simulationCase.geometrySetup.faceGroups.push_back(FaceGroup{"pipe.Default", "Default", "Pipe", "wall"});
    simulationCase.geometrySetup.faceGroups.push_back(FaceGroup{"pipe.Inlet1", "Inlet1", "Pipe", "inlet"});
    simulationCase.geometrySetup.faceGroups.push_back(FaceGroup{"pipe.Inlet2", "Inlet2", "Pipe", "inlet"});
    simulationCase.geometrySetup.faceGroups.push_back(FaceGroup{"pipe.Outlet", "Outlet", "Pipe", "outlet"});
    simulationCase.meshSetup.minimumSize = 0.0;
    simulationCase.meshSetup.maximumSize = 1.0;
    simulationCase.meshSetup.autoSize = true;
    simulationCase.meshSetup.localFaceGroupName = "pipe.Default";
    simulationCase.meshSetup.autoImportAfterGeneration = true;
    simulationCase.meshSetup.showBoundaryAfterImport = true;
    simulationCase.solverType = FlowSolverType::Simple;
    simulationCase.turbulenceModel = TurbulenceModel::KOmegaSST;
    simulationCase.runControl.endTime = 400.0;
    simulationCase.runControl.cleanPreviousResult = false;
    simulationCase.postProcessingTool = "ParaView";

    Material waterLikeFluid;
    waterLikeFluid.id = "fluid";
    waterLikeFluid.name = "Fluid";
    waterLikeFluid.domain = MaterialDomain::Fluid;
    waterLikeFluid.viscosityModel = ViscosityModel::Newtonian;
    simulationCase.materials.push_back(waterLikeFluid);

    BoundaryCondition defaultWall;
    defaultWall.id = "bc_default_wall";
    defaultWall.name = "pipe.Default";
    defaultWall.type = BoundaryConditionType::Wall;
    defaultWall.target.geometryName = "Pipe";
    defaultWall.target.faceGroupId = "pipe.Default";
    defaultWall.target.faceGroupName = "Default";
    defaultWall.materialId = waterLikeFluid.id;
    simulationCase.boundaryConditions.push_back(defaultWall);

    BoundaryCondition inlet1;
    inlet1.id = "bc_inlet1";
    inlet1.name = "pipe.Inlet1";
    inlet1.type = BoundaryConditionType::VelocityInlet;
    inlet1.target.geometryName = "Pipe";
    inlet1.target.faceGroupId = "pipe.Inlet1";
    inlet1.target.faceGroupName = "Inlet1";
    inlet1.materialId = waterLikeFluid.id;
    simulationCase.boundaryConditions.push_back(inlet1);

    BoundaryCondition inlet2;
    inlet2.id = "bc_inlet2";
    inlet2.name = "pipe.Inlet2";
    inlet2.type = BoundaryConditionType::PressureInlet;
    inlet2.target.geometryName = "Pipe";
    inlet2.target.faceGroupId = "pipe.Inlet2";
    inlet2.target.faceGroupName = "Inlet2";
    inlet2.materialId = waterLikeFluid.id;
    simulationCase.boundaryConditions.push_back(inlet2);

    BoundaryCondition outlet;
    outlet.id = "bc_outlet";
    outlet.name = "pipe.Outlet";
    outlet.type = BoundaryConditionType::PressureOutlet;
    outlet.target.geometryName = "Pipe";
    outlet.target.faceGroupId = "pipe.Outlet";
    outlet.target.faceGroupName = "Outlet";
    outlet.materialId = waterLikeFluid.id;
    simulationCase.boundaryConditions.push_back(outlet);

    Load inletVelocity;
    inletVelocity.id = "load_inlet1_velocity";
    inletVelocity.name = "Inlet1 velocity";
    inletVelocity.type = LoadType::Velocity;
    inletVelocity.boundaryConditionId = inlet1.id;
    inletVelocity.fieldName = "U";
    inletVelocity.value.kind = LoadValueKind::Scalar;
    inletVelocity.value.x = 0.8;
    simulationCase.loads.push_back(inletVelocity);

    Load inletPressure;
    inletPressure.id = "load_inlet2_pressure";
    inletPressure.name = "Inlet2 pressure";
    inletPressure.type = LoadType::Pressure;
    inletPressure.boundaryConditionId = inlet2.id;
    inletPressure.fieldName = "p";
    inletPressure.value.kind = LoadValueKind::Scalar;
    inletPressure.value.x = 1.0;
    simulationCase.loads.push_back(inletPressure);

    return simulationCase;
}
