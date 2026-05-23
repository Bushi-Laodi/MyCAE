#pragma once

#include "solver/SimulationCase.h"

class ProjectModel;

class SimulationCaseBuilder
{
public:
    static SimulationCase fromProjectModel(const ProjectModel &projectModel);
};
