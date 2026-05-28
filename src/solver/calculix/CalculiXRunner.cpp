#include "solver/calculix/CalculiXRunner.h"

#include "solver/calculix/CalculiXCasePaths.h"
#include "solver/calculix/CalculiXEnvironment.h"
#include "solver/calculix/CalculiXRunDiagnostics.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QTextStream>

namespace
{
constexpr int CalculiXRunTimeoutMs = 6 * 60 * 60 * 1000;

bool writeRunLog(const SolverRunResult &result, const QString &logFile, QString *errorMessage)
{
    QFile file(logFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write CalculiX log file: " + file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream << "Command: " << result.command << '\n';
    stream << "Working directory: " << result.workingDirectory << '\n';
    stream << "Exit code: " << result.exitCode << "\n\n";
    stream << "[stdout]\n" << result.standardOutput << "\n\n";
    stream << "[stderr]\n" << result.standardError << '\n';
    return true;
}

void tryWriteLog(SolverRunResult &result)
{
    QString logError;
    if (!writeRunLog(result, result.logFile, &logError)) {
        result.errors.append(logError);
    }
}

QStringList existingResultFiles(const CalculiXCasePaths &paths)
{
    QStringList files;
    for (const QString &path : {paths.staFile, paths.datFile, paths.frdFile}) {
        if (QFileInfo::exists(path)) {
            files.append(path);
        }
    }
    return files;
}

void appendDiagnosticReport(SolverRunResult &result, const CalculiXCasePaths &paths)
{
    const CalculiXRunDiagnosticReport report = CalculiXRunDiagnostics().analyze(paths, result);
    for (const QString &warning : report.warnings) {
        result.logMessages.append(warning);
    }
    for (const QString &hint : report.hints) {
        result.logMessages.append(hint);
    }
    for (const QString &error : report.errors) {
        if (!result.errors.contains(error)) {
            result.errors.append(error);
        }
    }
}
}

SolverRunResult CalculiXRunner::run(const SolverCaseContext &context) const
{
    SolverRunResult result;
    if (!context.isValid()) {
        result.errors.append("CalculiX run failed: invalid solver case context.");
        return result;
    }

    const CalculiXCasePaths paths = CalculiXCasePathsBuilder::fromContext(context);
    result.workingDirectory = paths.caseDirectory;
    result.logFile = paths.logFile;

    if (!QFileInfo::exists(paths.inputFile)) {
        result.errors.append("CalculiX run failed: input file does not exist: " + paths.inputFile);
        return result;
    }

    const QString program = CalculiXEnvironment::executablePath();
    if (CalculiXEnvironment::isExplicitExecutablePath(program) && !QFileInfo::exists(program)) {
        result.command = program + " " + paths.jobName;
        result.errors.append("CalculiX run failed: ccx executable does not exist: " + program);
        tryWriteLog(result);
        return result;
    }

    QProcess process;
    const QStringList arguments = {"-i", paths.jobName};
    process.setProgram(program);
    process.setArguments(arguments);
    process.setWorkingDirectory(paths.caseDirectory);
    process.setProcessEnvironment(CalculiXEnvironment::processEnvironment(program));
    result.command = program + " " + arguments.join(' ');

    process.start();
    if (!process.waitForStarted()) {
        result.standardError = process.errorString();
        result.errors.append("CalculiX run failed to start: " + process.errorString());
        tryWriteLog(result);
        return result;
    }

    if (!process.waitForFinished(CalculiXRunTimeoutMs)) {
        process.kill();
        process.waitForFinished();
        result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
        result.standardError = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        result.errors.append("CalculiX run timed out.");
        tryWriteLog(result);
        return result;
    }

    result.exitCode = process.exitCode();
    result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    result.standardError = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
    tryWriteLog(result);
    appendDiagnosticReport(result, paths);

    result.logMessages.append("CalculiX command: " + result.command);
    result.logMessages.append("CalculiX log: " + result.logFile);

    if (process.exitStatus() != QProcess::NormalExit) {
        result.errors.append("CalculiX process crashed or was terminated.");
        return result;
    }
    if (result.exitCode != 0) {
        result.errors.append(QString("CalculiX exited with code %1.").arg(result.exitCode));
        if (!result.standardError.isEmpty()) {
            result.errors.append(result.standardError);
        }
        return result;
    }
    if (existingResultFiles(paths).isEmpty()) {
        result.errors.append("CalculiX finished but no .sta, .dat or .frd result file was found.");
        return result;
    }

    result.success = result.errors.isEmpty();
    return result;
}
