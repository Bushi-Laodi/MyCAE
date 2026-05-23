#pragma once

#include "solver/plugin/SolverPlugin.h"

#include <memory>
#include <vector>

class SolverPluginManager
{
public:
    SolverPluginManager();

    const std::vector<std::unique_ptr<SolverPlugin>> &plugins() const;
    const SolverPlugin *pluginById(const QString &pluginId) const;

private:
    std::vector<std::unique_ptr<SolverPlugin>> m_plugins;
};
