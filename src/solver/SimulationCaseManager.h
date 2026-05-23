#pragma once

#include "project/Project.h"
#include "solver/SimulationCase.h"

#include <QString>

class ProjectModel;

class SimulationCaseManager
{
public:
    static QString relativeCaseFilePath();
    static QString caseFilePath(const Project &project);

    bool save(const ProjectModel &projectModel, QString *errorMessage = nullptr) const;
    bool load(const Project &project, SimulationCase &simulationCase, QString *errorMessage = nullptr) const;
    bool exists(const Project &project) const;
};
