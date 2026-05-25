#pragma once

#include <QString>

struct Project;
struct ResultObject;

struct ResultReportExportResult
{
    bool success = false;
    QString reportPath;
    QString errorMessage;
};

class ResultReportExporter
{
public:
    ResultReportExportResult exportMarkdown(
        const Project &project,
        const ResultObject &resultObject,
        const QString &reportPath,
        const QString &screenshotPath
    ) const;
};
