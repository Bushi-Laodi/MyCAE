#include "solver/calculix/CalculiXRunner.h"

#include "solver/calculix/CalculiXCasePaths.h"

#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTextStream>

namespace
{
constexpr int CalculiXRunTimeoutMs = 6 * 60 * 60 * 1000;

QString calculixExecutable()
{
    const QString fromEnvironment =
        QProcessEnvironment::systemEnvironment().value("MYCAE_CALCULIX_EXECUTABLE").trimmed();
    return fromEnvironment.isEmpty() ? QString("ccx") : fromEnvironment;
}

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

    QProcess process;
    const QString program = calculixExecutable();
    const QStringList arguments = {paths.jobName};
    process.setProgram(program);
    process.setArguments(arguments);
    process.setWorkingDirectory(paths.caseDirectory);
    result.command = program + " " + arguments.join(' ');

    process.start();
    if (!process.waitForStarted()) {
        result.standardError = process.errorString();
        result.errors.append("CalculiX run failed to start: " + process.errorString());
        return result;
    }

    if (!process.waitForFinished(CalculiXRunTimeoutMs)) {
        process.kill();
        process.waitForFinished();
        result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
        result.standardError = "Process timed out. "
            + QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        result.errors.append("CalculiX run timed out.");
        QString logError;
        writeRunLog(result, result.logFile, &logError);
        if (!logError.isEmpty()) {
            result.errors.append(logError);
        }
        return result;
    }

    result.exitCode = process.exitCode();
    result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    result.standardError = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();

    QString logError;
    if (!writeRunLog(result, result.logFile, &logError)) {
        result.errors.append(logError);
        return result;
    }

    result.logMessages.append("CalculiX command: " + result.command);
    result.logMessages.append("CalculiX log: " + result.logFile);
    if (process.exitStatus() == QProcess::NormalExit && result.exitCode == 0) {
        result.success = true;
        return result;
    }

    result.errors.append(QString("CalculiX run failed with exit code %1.").arg(result.exitCode));
    if (!result.standardError.isEmpty()) {
        result.errors.append(result.standardError);
    }
    return result;
}
