#include "result/ResultHistoryNormalizer.h"

#include "result/ResultObject.h"

#include <QDateTime>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QUuid>

namespace
{
QString sanitizedToken(QString value)
{
    value = value.trimmed().toLower();
    value.replace(QRegularExpression("[^a-z0-9_\\-]+"), "_");
    value.replace(QRegularExpression("_+"), "_");
    value = value.trimmed();
    return value.isEmpty() ? QString("result") : value;
}

QString caseDirectoryLabel(const QString &caseDirectory)
{
    const QString label = QFileInfo(caseDirectory).fileName().trimmed();
    return label.isEmpty() ? QFileInfo(caseDirectory).completeBaseName().trimmed() : label;
}

QString createdAtLabel(const QString &createdAt)
{
    const QDateTime dateTime = QDateTime::fromString(createdAt, Qt::ISODate);
    if (dateTime.isValid()) {
        return dateTime.toString("yyyyMMdd_HHmmss");
    }
    QString label = createdAt.trimmed();
    label.replace(QRegularExpression("[^0-9A-Za-z_\\-]+"), "_");
    return label;
}

QString runLabel(
    const QString &simulationCaseName,
    const QString &caseDirectory,
    const QString &createdAt
)
{
    const QString caseLabel = caseDirectoryLabel(caseDirectory);
    if (!caseLabel.isEmpty()) {
        return caseLabel;
    }

    const QString timeLabel = createdAtLabel(createdAt);
    if (!simulationCaseName.trimmed().isEmpty() && !timeLabel.isEmpty()) {
        return simulationCaseName.trimmed() + " - " + timeLabel;
    }
    if (!simulationCaseName.trimmed().isEmpty()) {
        return simulationCaseName.trimmed();
    }
    return timeLabel.isEmpty() ? QString("Run") : timeLabel;
}

bool isGenericResultName(const QString &name, const QString &solverName)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return true;
    }

    const QString solver = solverName.trimmed();
    return trimmed.compare("Result", Qt::CaseInsensitive) == 0
        || trimmed.compare(solver + " Result", Qt::CaseInsensitive) == 0
        || trimmed.compare("CalculiX Result", Qt::CaseInsensitive) == 0
        || trimmed.compare("OpenFOAM Result", Qt::CaseInsensitive) == 0;
}

QString uniqueName(QString baseName, const QSet<QString> &usedNames)
{
    if (!usedNames.contains(baseName)) {
        return baseName;
    }

    int suffix = 2;
    QString candidate;
    do {
        candidate = QString("%1 #%2").arg(baseName).arg(suffix++);
    } while (usedNames.contains(candidate));
    return candidate;
}
}

QString ResultHistoryNormalizer::makeResultId(const QString &pluginId)
{
    return sanitizedToken(pluginId) + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString ResultHistoryNormalizer::makeResultName(
    const QString &solverName,
    const QString &simulationCaseName,
    const QString &caseDirectory,
    const QString &createdAt
)
{
    const QString solver = solverName.trimmed().isEmpty() ? QString("Solver") : solverName.trimmed();
    return solver + " - " + runLabel(simulationCaseName, caseDirectory, createdAt);
}

QString ResultHistoryNormalizer::pluginIdForSolverName(const QString &solverName)
{
    if (solverName.contains("CalculiX", Qt::CaseInsensitive)) {
        return "calculix";
    }
    if (solverName.contains("OpenFOAM", Qt::CaseInsensitive)) {
        return "openfoam";
    }
    return sanitizedToken(solverName);
}

bool ResultHistoryNormalizer::normalize(std::vector<ResultObject> &results, QStringList *messages)
{
    bool changed = false;
    QSet<QString> usedIds;
    QSet<QString> usedNames;

    for (ResultObject &result : results) {
        const QString originalId = result.id;
        if (result.id.trimmed().isEmpty() || usedIds.contains(result.id)) {
            result.id = makeResultId(pluginIdForSolverName(result.solverName));
            changed = true;
            if (messages) {
                messages->append(QString("Result history normalized duplicate id: %1 -> %2.")
                    .arg(originalId)
                    .arg(result.id));
            }
        }
        usedIds.insert(result.id);

        QString normalizedName = result.name.trimmed();
        if (isGenericResultName(normalizedName, result.solverName)) {
            normalizedName = makeResultName(result.solverName, {}, result.casePath, result.createdAt);
        }
        normalizedName = uniqueName(normalizedName, usedNames);
        if (normalizedName != result.name) {
            result.name = normalizedName;
            changed = true;
        }
        usedNames.insert(result.name);
    }

    return changed;
}
