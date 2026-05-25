#pragma once

#include "solver/export/SolverCaseWriterResult.h"

#include <QString>

class ProjectModel;

class SolverCaseWriter
{
public:
    SolverCaseWriterResult writeOpenFoamCase(
        const ProjectModel &projectModel,
        const QString &caseName
    ) const;
};
