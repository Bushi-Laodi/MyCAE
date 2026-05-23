#include "solver/plugin/ExternalSolverPluginConfig.h"

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

QString ExternalSolverPluginConfigLoader::configPath(const QString &pluginDirectory)
{
    return QDir(pluginDirectory).filePath("solver_plugin.json");
}

QString ExternalSolverPluginConfigLoader::scriptPath(
    const QString &pluginDirectory,
    const ExternalSolverPluginConfig &config
)
{
    return QDir(pluginDirectory).filePath(config.script);
}

ExternalSolverPluginConfig ExternalSolverPluginConfigLoader::load(const QString &pluginDirectory)
{
    ExternalSolverPluginConfig config;

    QFile file(configPath(pluginDirectory));
    if (!file.open(QIODevice::ReadOnly)) {
        return config;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return config;
    }

    const QJsonObject object = document.object();
    config.id = object.value("id").toString(config.id);
    config.name = object.value("name").toString(config.name);
    config.type = object.value("type").toString(config.type);
    config.executable = object.value("executable").toString();
    config.script = object.value("script").toString();
    config.inputFile = object.value("inputFile").toString(config.inputFile);
    config.outputFile = object.value("outputFile").toString(config.outputFile);
    config.description = object.value("description").toString();

    const QJsonArray supportedAnalysisTypes = object.value("supportedAnalysisTypes").toArray();
    for (const QJsonValue &analysisType : supportedAnalysisTypes) {
        config.supportedAnalysisTypes.append(analysisType.toString());
    }

    return config;
}
