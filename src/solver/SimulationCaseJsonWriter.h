#pragma once

#include <QJsonDocument>
#include <QString>

struct SimulationCase;

class SimulationCaseJsonWriter
{
public:
    static QJsonDocument toJson(const SimulationCase &simulationCase);
    static bool writeCaseFile(
        const SimulationCase &simulationCase,
        const QString &filePath,
        QString *errorMessage = nullptr
    );
};
