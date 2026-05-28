#pragma once

#include "solver/BoundaryCondition.h"
#include "solver/BoundaryBindingInspector.h"
#include "solver/Load.h"
#include "solver/Material.h"
#include "solver/SimulationCase.h"

#include <vector>

class QWidget;

class SolverPropertyView
{
public:
    static void populateMaterialCategory(QWidget *parent, const std::vector<Material> &materials);
    static void populateBoundaryConditionCategory(
        QWidget *parent,
        const std::vector<BoundaryCondition> &boundaryConditions
    );
    static void populateLoadCategory(QWidget *parent, const std::vector<Load> &loads);
    static void populateMaterial(QWidget *parent, const Material &material);
    static void populateBoundaryCondition(
        QWidget *parent,
        const BoundaryCondition &boundaryCondition,
        const BoundaryConditionBindingSummary &bindingSummary
    );
    static void populateLoad(QWidget *parent, const Load &load);
    static void populateSolverCategory(QWidget *parent, const SimulationCase &simulationCase);
};
