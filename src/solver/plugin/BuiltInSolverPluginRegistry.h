#pragma once

#include "solver/plugin/SolverPlugin.h"

#include <memory>
#include <vector>

class BuiltInSolverPluginRegistry
{
public:
    static std::vector<std::unique_ptr<SolverPlugin>> createPlugins();
};
