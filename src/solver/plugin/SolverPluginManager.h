#pragma once

#include "solver/plugin/SolverPlugin.h"
#include "solver/plugin/SolverPluginDescriptor.h"

#include <QHash>
#include <QStringList>

#include <memory>
#include <vector>

class SolverPluginManager
{
public:
    SolverPluginManager();

    const std::vector<SolverPluginDescriptor> &pluginDescriptors() const;
    const SolverPlugin *pluginById(const QString &pluginId) const;
    const SolverPluginDescriptor *descriptorById(const QString &pluginId) const;
    QStringList diagnostics() const;

private:
    void registerPlugin(std::unique_ptr<SolverPlugin> plugin, const QString &source);

    std::vector<std::unique_ptr<SolverPlugin>> m_plugins;
    std::vector<SolverPluginDescriptor> m_descriptors;
    QHash<QString, const SolverPlugin *> m_pluginIndex;
    QHash<QString, int> m_descriptorIndex;
    QStringList m_diagnostics;
};
