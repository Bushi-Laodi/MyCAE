#include "solver/calculix/CalculiXResultReader.h"

#include "solver/calculix/CalculiXCasePaths.h"

#include <QFile>
#include <QFileInfo>
#include <QStringList>

namespace
{
QString readTail(const QString &filePath, qsizetype maxCharacters = 1200)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QString text = QString::fromLocal8Bit(file.readAll()).trimmed();
    if (text.size() <= maxCharacters) {
        return text;
    }
    return text.right(maxCharacters).trimmed();
}

void appendExistingFile(QStringList &files, const QString &filePath)
{
    if (QFileInfo::exists(filePath)) {
        files.append(filePath);
    }
}
}

SolverResultReadResult CalculiXResultReader::read(const SolverCaseContext &context) const
{
    SolverResultReadResult result;
    if (!context.isValid()) {
        result.errors.append("CalculiX result read failed: invalid solver case context.");
        return result;
    }

    const CalculiXCasePaths paths = CalculiXCasePathsBuilder::fromContext(context);
    appendExistingFile(result.resultFiles, paths.staFile);
    appendExistingFile(result.resultFiles, paths.datFile);
    appendExistingFile(result.resultFiles, paths.frdFile);
    appendExistingFile(result.resultFiles, paths.logFile);

    if (result.resultFiles.isEmpty()) {
        result.errors.append("CalculiX result read failed: no result files found in " + paths.caseDirectory);
        return result;
    }

    if (!QFileInfo::exists(paths.staFile)) {
        result.warnings.append("CalculiX .sta file was not found: " + paths.staFile);
    }
    if (!QFileInfo::exists(paths.datFile)) {
        result.warnings.append("CalculiX .dat file was not found: " + paths.datFile);
    }
    if (!QFileInfo::exists(paths.frdFile)) {
        result.warnings.append("CalculiX .frd file was not found: " + paths.frdFile);
    }

    const QString staTail = readTail(paths.staFile);
    const QString datTail = readTail(paths.datFile);
    QStringList summaryParts;
    summaryParts.append(QString("files=%1").arg(result.resultFiles.size()));
    if (!staTail.isEmpty()) {
        summaryParts.append("staTail=" + staTail.simplified().left(240));
    } else if (!datTail.isEmpty()) {
        summaryParts.append("datTail=" + datTail.simplified().left(240));
    }

    result.summary = "CalculiX result read: " + summaryParts.join("; ");
    result.logMessages.append("CalculiX result directory: " + paths.caseDirectory);
    result.success = true;
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>

namespace
{
QString readTextFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromLocal8Bit(file.readAll());
}

QString firstExistingText(const QStringList &paths)
{
    for (const QString &path : paths) {
        if (QFileInfo::exists(path)) {
            const QString text = readTextFile(path);
            if (!text.isEmpty()) {
                return text;
            }
        }
    }
    return {};
}

QStringList existingResultFiles(const QDir &caseDir, const QString &jobName)
{
    QStringList files;
    const QStringList suffixes{".sta", ".dat", ".frd"};
    for (const QString &suffix : suffixes) {
        const QString path = caseDir.filePath(jobName + suffix);
        if (QFileInfo::exists(path)) {
            files.append(path);
        }
    }
    return files;
}

bool containsFailureMarker(const QString &text)
{
    const QString upper = text.toUpper();
    return upper.contains("ERROR")
        || upper.contains("FAILED")
        || upper.contains("*ERROR")
        || upper.contains("TOO MANY CUTBACKS")
        || upper.contains("DIVERGENCE")
        || upper.contains("NO CONVERGENCE");
}

bool containsCompletionMarker(const QString &text)
{
    const QString upper = text.toUpper();
    return upper.contains("COMPLETED")
        || upper.contains("THE ANALYSIS HAS BEEN COMPLETED")
        || upper.contains("TOTAL TIME COMPLETED");
}

QString makeResultId()
{
    return "calculix_" + QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmsszzz");
}
}

CalculiXResultReadResult CalculiXResultReader::read(const QString &caseDirectory, const QString &jobName) const
{
    CalculiXResultReadResult result;
    const QString normalizedJobName = jobName.trimmed();
    if (normalizedJobName.isEmpty()) {
        result.errors.append("CalculiX result read failed: job name is empty.");
        return result;
    }

    const QDir caseDir(caseDirectory);
    if (!caseDir.exists()) {
        result.errors.append("CalculiX result read failed: case directory does not exist: " + caseDirectory);
        return result;
    }

    const QString staPath = caseDir.filePath(normalizedJobName + ".sta");
    const QString datPath = caseDir.filePath(normalizedJobName + ".dat");
    const QString frdPath = caseDir.filePath(normalizedJobName + ".frd");
    result.resultFiles = existingResultFiles(caseDir, normalizedJobName);

    if (result.resultFiles.isEmpty()) {
        result.errors.append("CalculiX result read failed: no .sta, .dat or .frd result file was found.");
        return result;
    }

    const QString statusText = readTextFile(staPath) + "\n" + readTextFile(datPath);
    const QString diagnosticText = firstExistingText(QStringList{datPath, staPath});
    if (containsFailureMarker(statusText)) {
        result.errors.append("CalculiX result contains failure markers. Check job.sta/job.dat for details.");
    }
    if (!QFileInfo::exists(staPath)) {
        result.warnings.append("CalculiX status file is missing: " + staPath);
    }
    if (!QFileInfo::exists(datPath)) {
        result.warnings.append("CalculiX data file is missing: " + datPath);
    }
    if (!QFileInfo::exists(frdPath)) {
        result.warnings.append("CalculiX FRD file is missing: " + frdPath);
    }

    const bool completed = containsCompletionMarker(statusText);
    if (!completed && result.errors.isEmpty()) {
        result.warnings.append("CalculiX result files exist, but completion could not be confirmed from .sta/.dat.");
    }

    result.summary = completed
        ? "CalculiX analysis completed."
        : "CalculiX result files detected; completion status is uncertain.";
    if (!result.errors.isEmpty()) {
        result.summary = "CalculiX analysis failed or did not converge.";
    } else if (!diagnosticText.isEmpty()) {
        const QString firstLine = diagnosticText.split('\n', Qt::SkipEmptyParts).value(0).trimmed();
        if (!firstLine.isEmpty()) {
            result.logMessages.append("CalculiX diagnostic: " + firstLine);
        }
    }

    ResultObject object;
    object.id = makeResultId();
    object.name = "CalculiX Result";
    object.solverName = "CalculiX";
    object.casePath = caseDirectory;
    object.logFile = caseDir.filePath(normalizedJobName + ".ccx.log");
    object.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    object.success = result.errors.isEmpty();
    object.summary = result.summary;
    result.resultObject = object;

    result.logMessages.append(result.summary);
    result.success = result.errors.isEmpty();
    return result;
}
