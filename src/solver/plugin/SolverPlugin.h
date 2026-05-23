#pragma once

#include <QString>

struct SimulationCase;

class SolverPlugin
{
public:
    virtual ~SolverPlugin() = default;

    virtual QString id() const = 0;
    virtual QString name() const = 0;

    virtual bool exportCase(
        const SimulationCase &simulationCase,
        const QString &caseDirectory,
        QString *errorMessage = nullptr
    ) const = 0;

    virtual bool runCase(
        const QString &caseDirectory,
        QString *logText = nullptr,
        QString *errorMessage = nullptr
    ) const = 0;

    virtual bool readResult(
        const QString &caseDirectory,
        QString *resultText = nullptr,
        QString *errorMessage = nullptr
    ) const = 0;
};
