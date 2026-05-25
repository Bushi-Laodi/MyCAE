#pragma once

#include <QString>
#include <QVector>

struct UiValidationStep
{
    QString name;
    bool passed = false;
    QString detail;
};

struct UiValidationReport
{
    QVector<UiValidationStep> steps;

    bool success() const;
    int passedCount() const;
    int failedCount() const;
};

class UiSmokeValidator
{
public:
    UiValidationReport validate() const;
};

QString uiValidationStatusName(bool passed);
