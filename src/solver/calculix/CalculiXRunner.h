#pragma once

#include <QString>
#include <QStringList>

struct CalculiXRunResult
{
    bool success = false;
    int exitCode = -1;
    QString jobName;
    QString command;
    QString workingDirectory;
    QString standardOutput;
    QString standardError;
    QString logFile;
    QStringList resultFiles;
    QStringList errors;
    QStringList logMessages;
};

class CalculiXRunner
{
public:
    explicit CalculiXRunner(QString executablePath = {});

    CalculiXRunResult run(const QString &caseDirectory, const QString &jobName = "job") const;

    QString executablePath() const;

private:
    QString m_executablePath;
};
