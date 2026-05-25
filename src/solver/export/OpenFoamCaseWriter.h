#pragma once

#include "solver/export/SolverCaseWriterResult.h"

#include <QString>

class ProjectModel;
struct SimulationCase;

class OpenFoamCaseWriter
{
public:
    SolverCaseWriterResult write(
        const ProjectModel &projectModel,
        const SimulationCase &simulationCase,
        const QString &caseName
    ) const;
};
