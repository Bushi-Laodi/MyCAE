#pragma once

#include <QString>

struct SampleValidationReport;

class DemoProjectValidator
{
public:
    explicit DemoProjectValidator(QString samplesRoot);

    void validate(SampleValidationReport &report) const;

private:
    QString m_samplesRoot;
};
