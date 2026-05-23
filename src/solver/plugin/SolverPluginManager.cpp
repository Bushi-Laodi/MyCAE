#include "solver/plugin/SolverPluginManager.h"

#include "solver/plugin/ExternalProcessSolverPlugin.h"
#include "solver/plugin/ExternalSolverPluginConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

namespace
{
QString solverPluginRootDirectory()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath("resource/solver");
}
}

SolverPluginManager::SolverPluginManager()
{
    const QDir pluginRoot(solverPluginRootDirectory());
    const QFileInfoList pluginDirectories = pluginRoot.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &pluginDirectory : pluginDirectories) {
        const QString configPath = ExternalSolverPluginConfigLoader::configPath(pluginDirectory.absoluteFilePath());
        if (QFileInfo::exists(configPath)) {
            m_plugins.push_back(std::make_unique<ExternalProcessSolverPlugin>(pluginDirectory.absoluteFilePath()));
        }
    }
}

const std::vector<std::unique_ptr<SolverPlugin>> &SolverPluginManager::plugins() const
{
    return m_plugins;
}

const SolverPlugin *SolverPluginManager::pluginById(const QString &pluginId) const
{
    for (const std::unique_ptr<SolverPlugin> &plugin : m_plugins) {
        if (plugin->id() == pluginId) {
            return plugin.get();
        }
    }
    return nullptr;
}
