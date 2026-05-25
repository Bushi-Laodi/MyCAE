#include "solver/calculix/CalculiXRunner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QProcess>
#include <QTextStream>

namespace
{
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
