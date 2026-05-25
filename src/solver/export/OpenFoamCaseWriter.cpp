#include "solver/export/OpenFoamCaseWriter.h"

SolverCaseWriterResult OpenFoamCaseWriter::write(const SolverCaseContext &context) const
{
    SolverCaseWriterResult result;
    result.errors.append("OpenFOAM case export is not implemented yet.");
    result.logMessages.append("OpenFOAM case export deferred: " + context.caseName);
    return result;
}
