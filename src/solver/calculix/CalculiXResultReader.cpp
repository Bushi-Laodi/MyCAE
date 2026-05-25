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
    return result;
}
