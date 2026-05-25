#pragma once

#include <QVector>
#include <QString>
#include <QStringList>

enum class SampleValidationStatus
{
    Pass,
    Fail,
    Skip
};

struct SampleValidationStep
{
    SampleValidationStatus status = SampleValidationStatus::Skip;
    QString name;
    QString detail;
};

struct SampleValidationReport
{
    QVector<SampleValidationStep> steps;
    QStringList logMessages;

    void addStep(SampleValidationStatus status, const QString &name, const QString &detail = {});
    int passedCount() const;
    int failedCount() const;
    bool success() const;
};

QString sampleValidationStatusName(SampleValidationStatus status);

class SampleProjectValidator
{
public:
    explicit SampleProjectValidator(QString samplesRoot = {}, QString workRoot = {});

    SampleValidationReport validate() const;

    static QString defaultSamplesRoot();
    static QString defaultWorkRoot();

private:
    QString m_samplesRoot;
    QString m_workRoot;
};
