#include "solver/calculix/CalculiXResultReader.h"

#include "solver/calculix/CalculiXCasePaths.h"

#include <QFile>
#include <QFileInfo>
#include <QStringList>

namespace
{
QString readTextFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromLocal8Bit(file.readAll()).trimmed();
}

QString readTail(const QString &filePath, qsizetype maxCharacters = 1200)
{
    const QString text = readTextFile(filePath);
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

    const QString statusText = readTextFile(paths.staFile) + "\n" + readTextFile(paths.datFile);
    if (containsFailureMarker(statusText)) {
        result.errors.append("CalculiX result contains failure markers. Check .sta/.dat for details.");
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

    const bool completed = containsCompletionMarker(statusText);
    if (!completed && result.errors.isEmpty()) {
        result.warnings.append("CalculiX result files exist, but completion could not be confirmed from .sta/.dat.");
    }

    const QString staTail = readTail(paths.staFile);
    const QString datTail = readTail(paths.datFile);
    QStringList summaryParts;
    summaryParts.append(QString("files=%1").arg(result.resultFiles.size()));
    summaryParts.append(completed ? "completed=yes" : "completed=unknown");
    if (!staTail.isEmpty()) {
        summaryParts.append("staTail=" + staTail.simplified().left(240));
    } else if (!datTail.isEmpty()) {
        summaryParts.append("datTail=" + datTail.simplified().left(240));
    }

    result.summary = result.errors.isEmpty()
        ? "CalculiX result read: " + summaryParts.join("; ")
        : "CalculiX analysis failed or did not converge.";
    result.logMessages.append("CalculiX result directory: " + paths.caseDirectory);
    result.success = result.errors.isEmpty();
    return result;
}
