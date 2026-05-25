#include "solver/calculix/CalculiXRunner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QProcess>
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
QString defaultCalculiXExecutable()
{
#ifdef MYCAE_CALCULIX_EXECUTABLE_PATH
    const QString configuredPath = QString::fromUtf8(MYCAE_CALCULIX_EXECUTABLE_PATH).trimmed();
    if (!configuredPath.isEmpty()) {
        return configuredPath;
    }
#endif
    return "ccx";
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

bool isExplicitExecutablePath(const QString &executablePath)
{
    const QFileInfo executableInfo(executablePath);
    return executableInfo.isAbsolute() || executablePath.contains('/') || executablePath.contains('\\');
}

void writeRunLog(CalculiXRunResult *result)
{
    if (!result) {
        return;
    }

    const QString logFileName = result->jobName.isEmpty() ? "job.ccx.log" : result->jobName + ".ccx.log";
    const QString logPath = QDir(result->workingDirectory).filePath(logFileName);
    QFile logFile(logPath);
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result->logMessages.append("Failed to write CalculiX log file: " + logFile.errorString());
        return;
    }

    QTextStream stream(&logFile);
    stream << "Command: " << result->command << '\n';
    stream << "Working directory: " << result->workingDirectory << '\n';
    stream << "Exit code: " << result->exitCode << "\n\n";
    stream << "[stdout]\n" << result->standardOutput << "\n\n";
    stream << "[stderr]\n" << result->standardError << '\n';
    result->logFile = logPath;
}
}

CalculiXRunner::CalculiXRunner(QString executablePath)
    : m_executablePath(executablePath.trimmed().isEmpty() ? defaultCalculiXExecutable() : executablePath.trimmed())
{
}

CalculiXRunResult CalculiXRunner::run(const QString &caseDirectory, const QString &jobName) const
{
    CalculiXRunResult result;
    const QString normalizedJobName = jobName.trimmed();
    result.jobName = normalizedJobName;
    result.workingDirectory = caseDirectory;
    result.command = m_executablePath + " " + normalizedJobName;

    if (normalizedJobName.isEmpty()) {
        result.errors.append("CalculiX run failed: job name is empty.");
        return result;
    }

    const QDir caseDir(caseDirectory);
    if (!caseDir.exists()) {
        result.errors.append("CalculiX run failed: case directory does not exist: " + caseDirectory);
        return result;
    }

    if (isExplicitExecutablePath(m_executablePath) && !QFileInfo::exists(m_executablePath)) {
        result.errors.append("CalculiX run failed: ccx executable does not exist: " + m_executablePath);
        writeRunLog(&result);
        return result;
    }

    const QString inputPath = caseDir.filePath(normalizedJobName + ".inp");
    if (!QFileInfo::exists(inputPath)) {
        result.errors.append("CalculiX run failed: input deck does not exist: " + inputPath);
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
    process.setProgram(m_executablePath);
    process.setArguments({normalizedJobName});
    process.setWorkingDirectory(caseDirectory);
    process.start();

    if (!process.waitForStarted()) {
        result.standardError = process.errorString();
        result.errors.append("CalculiX run failed to start: " + process.errorString());
        writeRunLog(&result);
        return result;
    }

    constexpr int timeoutMs = 30 * 60 * 1000;
    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished();
        result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
        result.standardError = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        result.errors.append("CalculiX run timed out.");
        writeRunLog(&result);
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
    result.resultFiles = existingResultFiles(caseDir, normalizedJobName);
    writeRunLog(&result);

    if (process.exitStatus() != QProcess::NormalExit) {
        result.errors.append("CalculiX process crashed or was terminated.");
        return result;
    }

    if (result.exitCode != 0) {
        result.errors.append(QString("CalculiX exited with code %1.").arg(result.exitCode));
        return result;
    }

    if (result.resultFiles.isEmpty()) {
        result.errors.append("CalculiX finished but no .sta, .dat or .frd result file was found.");
        return result;
    }

    result.logMessages.append("CalculiX run finished: " + result.command);
    result.success = true;
    return result;
}

QString CalculiXRunner::executablePath() const
{
    return m_executablePath;
}
