#include "solver/export/SolverCaseWriter.h"

#include "project/ProjectModel.h"
#include "solver/SimulationCaseBuilder.h"
#include "solver/export/OpenFoamCaseWriter.h"

SolverCaseWriterResult SolverCaseWriter::writeOpenFoamCase(
    const ProjectModel &projectModel,
    const QString &caseName
) const
{
    const SimulationCase simulationCase = SimulationCaseBuilder::fromProjectModel(projectModel);
    const OpenFoamCaseWriter openFoamCaseWriter;
    return openFoamCaseWriter.write(projectModel, simulationCase, caseName);
}
