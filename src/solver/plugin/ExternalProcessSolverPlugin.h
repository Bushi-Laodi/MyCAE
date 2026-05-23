#pragma once

#include "solver/plugin/SolverPlugin.h"

class ExternalProcessSolverPlugin final : public SolverPlugin
{
public:
    explicit ExternalProcessSolverPlugin(QString pluginDirectory);

    QString id() const override;
    QString name() const override;

    bool exportCase(
        const SimulationCase &simulationCase,
        const QString &caseDirectory,
        QString *errorMessage = nullptr
    ) const override;

    bool runCase(
        const QString &caseDirectory,
        QString *logText = nullptr,
        QString *errorMessage = nullptr
    ) const override;

    bool readResult(
        const QString &caseDirectory,
        QString *resultText = nullptr,
        QString *errorMessage = nullptr
    ) const override;

private:
    QString m_pluginDirectory;
};
