#pragma once

#include "solver/plugin/SolverCaseContext.h"

#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QUrl>

struct OpenFoamServiceRunResult
{
    bool success = false;
    int httpStatus = 0;
    QString serviceUrl;
    QString jobId;
    QString summary;
    QString logFile;
    QStringList logMessages;
    QStringList vtkFiles;
    QStringList errors;
    QJsonObject response;
};

class OpenFoamServiceClient
{
public:
    static QUrl defaultBaseUrl();
    static QString responseFilePath(const QString &caseDirectory);
    static QString serviceLogFilePath(const QString &caseDirectory);
    static QString requestManifestPath(const QString &caseDirectory);
    static QString startCommandHint();

    explicit OpenFoamServiceClient(QUrl baseUrl = defaultBaseUrl());

    OpenFoamServiceRunResult runDemoCase(const SolverCaseContext &context) const;

private:
    QJsonObject buildRequest(const SolverCaseContext &context) const;

    QUrl m_baseUrl;
};
