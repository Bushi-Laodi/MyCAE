#pragma once

#include <QString>
#include <QStringList>

struct CalculiXDatResult;
struct MeshData;

struct ResultDisplaySummary
{
    bool success = false;
    QString scalarName;
    int matchedNodeCount = 0;
    int meshNodeCount = 0;
    int matchedElementCount = 0;
    int meshElementCount = 0;
    double scalarMin = 0.0;
    double scalarMax = 0.0;
    QStringList warnings;
    QStringList errors;
};

class ResultDisplaySummarizer
{
public:
    ResultDisplaySummary summarize(
        const MeshData &meshData,
        const CalculiXDatResult &result,
        const QString &fieldName
    ) const;
};
