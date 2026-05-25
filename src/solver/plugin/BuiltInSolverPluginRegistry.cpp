#include "solver/plugin/BuiltInSolverPluginRegistry.h"

#include "solver/calculix/CalculiXPlugin.h"
#include "solver/openfoam/OpenFoamPlugin.h"

std::vector<std::unique_ptr<SolverPlugin>> BuiltInSolverPluginRegistry::createPlugins()
{
    std::vector<std::unique_ptr<SolverPlugin>> plugins;
    plugins.push_back(std::make_unique<CalculiXPlugin>());
    plugins.push_back(std::make_unique<OpenFoamPlugin>());
    return plugins;
}
