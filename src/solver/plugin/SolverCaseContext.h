#pragma once

#include <QString>

class ProjectModel;
struct SimulationCase;

struct SolverCaseContext
{
    const ProjectModel *projectModel = nullptr;
    const SimulationCase *simulationCase = nullptr;
    QString caseDirectory;
    QString caseName;

    bool isValid() const
    {
        return projectModel && simulationCase && !caseDirectory.isEmpty();
    }
};
