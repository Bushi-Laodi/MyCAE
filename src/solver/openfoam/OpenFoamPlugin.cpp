#include "solver/openfoam/OpenFoamPlugin.h"

#include "solver/export/OpenFoamCaseWriter.h"

SolverPluginDescriptor OpenFoamPlugin::descriptor() const
{
    SolverPluginDescriptor descriptor;
    descriptor.id = "openfoam";
    descriptor.name = "OpenFOAM";
    descriptor.vendor = "OpenFOAM Foundation / OpenCFD";
    descriptor.version = "reserved";
    descriptor.solverFamily = "cfd";
    descriptor.description = "Reserved built-in OpenFOAM integration point.";
    descriptor.status = SolverPluginStatus::Reserved;
    descriptor.capabilities.canExportCase = false;
    descriptor.capabilities.canRunCase = false;
    descriptor.capabilities.canReadResult = false;
    descriptor.capabilities.analysisTypes = {"incompressibleFlow"};
    descriptor.capabilities.inputFormats = {"OpenFOAM case"};
    descriptor.capabilities.outputFormats = {"OpenFOAM time directories"};
    return descriptor;
}

SolverCaseWriterResult OpenFoamPlugin::exportCase(const SolverCaseContext &context) const
{
    const OpenFoamCaseWriter writer;
    return writer.write(context);
}

SolverRunResult OpenFoamPlugin::runCase(const SolverCaseContext &context) const
{
    Q_UNUSED(context);

    SolverRunResult result;
    result.errors.append("OpenFOAM run is reserved but not implemented in this stage.");
    result.logMessages.append("OpenFOAM placeholder reached run boundary.");
    return result;
}

SolverResultReadResult OpenFoamPlugin::readResult(const SolverCaseContext &context) const
{
    Q_UNUSED(context);

    SolverResultReadResult result;
    result.errors.append("OpenFOAM result reader is reserved but not implemented in this stage.");
    result.logMessages.append("OpenFOAM placeholder reached result read boundary.");
    return result;
}
