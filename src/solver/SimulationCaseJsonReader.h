#pragma once

#include "solver/SimulationCase.h"

#include <QJsonDocument>
#include <QString>

class SimulationCaseJsonReader
{
public:
    static bool fromJson(
        const QJsonDocument &document,
        SimulationCase &simulationCase,
        QString *errorMessage = nullptr
    );

    static bool readCaseFile(
        const QString &filePath,
        SimulationCase &simulationCase,
        QString *errorMessage = nullptr
    );
};
