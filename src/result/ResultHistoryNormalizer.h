#pragma once

#include <QString>
#include <QStringList>

#include <vector>

struct ResultObject;

class ResultHistoryNormalizer
{
public:
    static QString makeResultId(const QString &pluginId);
    static QString makeResultName(
        const QString &solverName,
        const QString &simulationCaseName,
        const QString &caseDirectory,
        const QString &createdAt
    );
    static QString pluginIdForSolverName(const QString &solverName);
    static bool normalize(std::vector<ResultObject> &results, QStringList *messages = nullptr);
};
