#include "solver/export/OpenFoamCaseWriter.h"

#include "project/ProjectModel.h"
#include "solver/SimulationCase.h"

SolverCaseWriterResult OpenFoamCaseWriter::write(
    const ProjectModel &projectModel,
    const SimulationCase &simulationCase,
    const QString &caseName
) const
{
    Q_UNUSED(projectModel);
    Q_UNUSED(simulationCase);

    SolverCaseWriterResult result;
    result.errors.append("OpenFOAM case export is not implemented yet.");
    result.logMessages.append("OpenFOAM case export deferred: " + caseName);
    return result;
}
