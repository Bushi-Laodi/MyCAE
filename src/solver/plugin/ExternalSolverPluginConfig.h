#pragma once

#include <QString>
#include <QStringList>

struct ExternalSolverPluginConfig
{
    QString id = "external";
    QString name = "External Solver";
    QString type = "python";
    QString executable;
    QString script;
    QString inputFile = "case.json";
    QString outputFile = "result.json";
    QString description;
    QStringList supportedAnalysisTypes;
};

class ExternalSolverPluginConfigLoader
{
public:
    static QString configPath(const QString &pluginDirectory);
    static QString scriptPath(const QString &pluginDirectory, const ExternalSolverPluginConfig &config);
    static ExternalSolverPluginConfig load(const QString &pluginDirectory);
};
