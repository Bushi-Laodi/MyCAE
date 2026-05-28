#pragma once

#include <QString>
#include <QStringList>

class ProjectModel;
struct ResultObject;

struct ResultCsvExportResult
{
    bool success = false;
    QString nodeDisplacementCsvPath;
    QString elementStressCsvPath;
    QString summaryCsvPath;
    QString metadataJsonPath;
    QStringList warnings;
    QString errorMessage;
};

class ResultCsvExporter
{
public:
    ResultCsvExportResult exportResult(
        const ProjectModel &projectModel,
        const ResultObject &resultObject,
        const QString &nodeDisplacementCsvPath
    ) const;
};
