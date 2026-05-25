#include "solver/export/SolverCaseWriter.h"

#include "solver/plugin/SolverPlugin.h"

SolverCaseWriterResult SolverCaseWriter::writeCase(const SolverPlugin &plugin, const SolverCaseContext &context) const
{
    return plugin.exportCase(context);
}
