#include "solver/calculix/CalculiXPlugin.h"

#include "solver/calculix/CalculiXCaseWriter.h"
#include "solver/calculix/CalculiXResultReader.h"
#include "solver/calculix/CalculiXRunner.h"

SolverPluginDescriptor CalculiXPlugin::descriptor() const
{
    SolverPluginDescriptor descriptor;
    descriptor.id = "calculix";
    descriptor.name = "CalculiX";
    descriptor.vendor = "CalculiX";
    descriptor.version = "reserved";
    descriptor.solverFamily = "structural";
    descriptor.description = "Built-in CalculiX integration for structural finite element analysis.";
    descriptor.status = SolverPluginStatus::Ready;
    descriptor.capabilities.canExportCase = true;
    descriptor.capabilities.canRunCase = true;
    descriptor.capabilities.canReadResult = true;
    descriptor.capabilities.analysisTypes = {"staticStructural"};
    descriptor.capabilities.inputFormats = {"inp"};
    descriptor.capabilities.outputFormats = {"dat", "sta", "frd"};
    return descriptor;
}

SolverCaseWriterResult CalculiXPlugin::exportCase(const SolverCaseContext &context) const
{
    const CalculiXCaseWriter writer;
    return writer.write(context);
}

SolverRunResult CalculiXPlugin::runCase(const SolverCaseContext &context) const
{
    const CalculiXRunner runner;
    return runner.run(context);
}

SolverResultReadResult CalculiXPlugin::readResult(const SolverCaseContext &context) const
{
    const CalculiXResultReader reader;
    return reader.read(context);
}
