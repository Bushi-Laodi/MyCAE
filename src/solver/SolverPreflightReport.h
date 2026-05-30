#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

enum class PreflightCheckStatus
{
    Passed,
    Warning,
    Failed
};

struct PreflightCheckItem
{
    QString category;
    PreflightCheckStatus status = PreflightCheckStatus::Passed;
    QString message;
    QString suggestion;
};

struct SolverPreflightReport
{
    QVector<PreflightCheckItem> items;

    bool hasFailed() const
    {
        for (const PreflightCheckItem &item : items) {
            if (item.status == PreflightCheckStatus::Failed) {
                return true;
            }
        }
        return false;
    }

    bool hasWarning() const
    {
        for (const PreflightCheckItem &item : items) {
            if (item.status == PreflightCheckStatus::Warning) {
                return true;
            }
        }
        return false;
    }

    QString summary() const
    {
        if (hasFailed()) {
            return QString::fromUtf8(u8"求解前检查未通过");
        }
        if (hasWarning()) {
            return QString::fromUtf8(u8"有警告，建议检查后继续");
        }
        return QString::fromUtf8(u8"求解前检查通过");
    }
};

inline QString toDisplayString(PreflightCheckStatus status)
{
    switch (status) {
    case PreflightCheckStatus::Passed:
        return QString::fromUtf8(u8"通过");
    case PreflightCheckStatus::Warning:
        return QString::fromUtf8(u8"警告");
    case PreflightCheckStatus::Failed:
        return QString::fromUtf8(u8"失败");
    }
    return QString::fromUtf8(u8"通过");
}
