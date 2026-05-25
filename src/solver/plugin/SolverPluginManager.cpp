#include "solver/plugin/SolverPluginManager.h"

#include "solver/plugin/BuiltInSolverPluginRegistry.h"
#include "solver/plugin/ExternalProcessSolverPlugin.h"
#include "solver/plugin/ExternalSolverPluginConfig.h"
#include "solver/plugin/SolverPluginDescriptorFormatter.h"

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
    for (std::unique_ptr<SolverPlugin> &plugin : BuiltInSolverPluginRegistry::createPlugins()) {
        registerPlugin(std::move(plugin), "built-in");
    }

    const QDir pluginRoot(solverPluginRootDirectory());
    if (!pluginRoot.exists()) {
        m_diagnostics.append("External solver plugin directory does not exist: " + pluginRoot.absolutePath());
        return;
    }

    const QFileInfoList pluginDirectories = pluginRoot.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &pluginDirectory : pluginDirectories) {
        const QString configPath = ExternalSolverPluginConfigLoader::configPath(pluginDirectory.absoluteFilePath());
        if (QFileInfo::exists(configPath)) {
            registerPlugin(
                std::make_unique<ExternalProcessSolverPlugin>(pluginDirectory.absoluteFilePath()),
                "external: " + pluginDirectory.absoluteFilePath()
            );
        }
    }
}

const std::vector<SolverPluginDescriptor> &SolverPluginManager::pluginDescriptors() const
{
    return m_descriptors;
}

const SolverPlugin *SolverPluginManager::pluginById(const QString &pluginId) const
{
    return m_pluginIndex.value(pluginId.trimmed(), nullptr);
}

const SolverPluginDescriptor *SolverPluginManager::descriptorById(const QString &pluginId) const
{
    const auto iterator = m_descriptorIndex.constFind(pluginId.trimmed());
    if (iterator == m_descriptorIndex.constEnd()) {
        return nullptr;
    }
    return &m_descriptors.at(iterator.value());
}

QStringList SolverPluginManager::diagnostics() const
{
    return m_diagnostics;
}

void SolverPluginManager::registerPlugin(std::unique_ptr<SolverPlugin> plugin, const QString &source)
{
    if (!plugin) {
        m_diagnostics.append("Skipped null solver plugin from " + source + ".");
        return;
    }

    SolverPluginDescriptor descriptor = plugin->descriptor();
    descriptor.id = descriptor.id.trimmed();
    if (descriptor.id.isEmpty()) {
        m_diagnostics.append("Skipped solver plugin with empty id from " + source + ".");
        return;
    }

    if (m_pluginIndex.contains(descriptor.id)) {
        m_diagnostics.append(
            QString("Skipped duplicate solver plugin id '%1' from %2.").arg(descriptor.id, source)
        );
        return;
    }

    m_diagnostics.append(SolverPluginDescriptorFormatter::registrationLog(descriptor, source));
    SolverPlugin *pluginPointer = plugin.get();
    m_pluginIndex.insert(descriptor.id, pluginPointer);
    m_descriptorIndex.insert(descriptor.id, static_cast<int>(m_descriptors.size()));
    m_descriptors.push_back(descriptor);
    m_plugins.push_back(std::move(plugin));
}
