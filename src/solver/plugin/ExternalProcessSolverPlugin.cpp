#include "solver/plugin/ExternalProcessSolverPlugin.h"

#include "solver/SimulationCaseJsonWriter.h"
#include "solver/plugin/ExternalSolverPluginConfig.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStringList>

#include <utility>
#include <vector>

namespace
{
bool runProcess(
    const QString &program,
    const QStringList &arguments,
    const QString &workingDirectory,
    QString *standardOutput,
    QString *standardError
)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.setWorkingDirectory(workingDirectory);
    process.start();

    if (!process.waitForStarted()) {
        if (standardError) {
            *standardError = process.errorString();
        }
        return false;
    }

    if (!process.waitForFinished(30000)) {
        process.kill();
        process.waitForFinished();
        if (standardOutput) {
            *standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
        }
        if (standardError) {
            *standardError = "Process timed out. " + QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        }
        return false;
    }

    if (standardOutput) {
        *standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    }
    if (standardError) {
        *standardError = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
    }
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

struct ProcessCommand
{
    QString program;
    QStringList arguments;
};

std::vector<ProcessCommand> commandsForConfig(
    const ExternalSolverPluginConfig &config,
    const QString &pluginDirectory,
    const QString &caseFilePath,
    const QString &resultFilePath
)
{
    const QString resolvedScriptPath = ExternalSolverPluginConfigLoader::scriptPath(pluginDirectory, config);

    if (!config.executable.isEmpty()) {
        QStringList arguments;
        if (!config.script.isEmpty()) {
            arguments.append(resolvedScriptPath);
        }
        arguments.append(caseFilePath);
        arguments.append(resultFilePath);
        return {{config.executable, arguments}};
    }

    if (config.type.compare("python", Qt::CaseInsensitive) == 0) {
        return {
            {"python", {resolvedScriptPath, caseFilePath, resultFilePath}},
            {"python3", {resolvedScriptPath, caseFilePath, resultFilePath}},
            {"py", {"-3", resolvedScriptPath, caseFilePath, resultFilePath}},
        };
    }

    return {};
}
}

ExternalProcessSolverPlugin::ExternalProcessSolverPlugin(QString pluginDirectory)
    : m_pluginDirectory(std::move(pluginDirectory))
{
}

QString ExternalProcessSolverPlugin::id() const
{
    return ExternalSolverPluginConfigLoader::load(m_pluginDirectory).id;
}

QString ExternalProcessSolverPlugin::name() const
{
    return ExternalSolverPluginConfigLoader::load(m_pluginDirectory).name;
}

bool ExternalProcessSolverPlugin::exportCase(
    const SimulationCase &simulationCase,
    const QString &caseDirectory,
    QString *errorMessage
) const
{
    const ExternalSolverPluginConfig config = ExternalSolverPluginConfigLoader::load(m_pluginDirectory);
    const QString caseFilePath = QDir(caseDirectory).filePath(config.inputFile);
    return SimulationCaseJsonWriter::writeCaseFile(simulationCase, caseFilePath, errorMessage);
}

bool ExternalProcessSolverPlugin::runCase(
    const QString &caseDirectory,
    QString *logText,
    QString *errorMessage
) const
{
    const ExternalSolverPluginConfig config = ExternalSolverPluginConfigLoader::load(m_pluginDirectory);
    const QString caseFilePath = QDir(caseDirectory).filePath(config.inputFile);
    if (!QFileInfo::exists(caseFilePath)) {
        if (errorMessage) {
            *errorMessage = "External solver input does not exist: " + caseFilePath;
        }
        return false;
    }

    const QString pluginConfigPath = ExternalSolverPluginConfigLoader::configPath(m_pluginDirectory);
    if (!QFileInfo::exists(pluginConfigPath)) {
        if (errorMessage) {
            *errorMessage = "External solver plugin description does not exist: " + pluginConfigPath;
        }
        return false;
    }

    if (!config.script.isEmpty()
            && !QFileInfo::exists(ExternalSolverPluginConfigLoader::scriptPath(m_pluginDirectory, config))) {
        if (errorMessage) {
            *errorMessage = "External solver script does not exist: "
                + ExternalSolverPluginConfigLoader::scriptPath(m_pluginDirectory, config);
        }
        return false;
    }

    const QString resultFilePath = QDir(caseDirectory).filePath(config.outputFile);
    const std::vector<ProcessCommand> commands = commandsForConfig(config, m_pluginDirectory, caseFilePath, resultFilePath);
    if (commands.empty()) {
        if (errorMessage) {
            *errorMessage = "External solver plugin has no runnable command: " + pluginConfigPath;
        }
        return false;
    }

    QStringList failureMessages;
    for (const ProcessCommand &command : commands) {
        QString standardOutput;
        QString standardError;
        if (runProcess(command.program, command.arguments, caseDirectory, &standardOutput, &standardError)) {
            if (logText) {
                *logText = QString("External solver plugin: %1 (%2)\nConfig: %3\nCommand: %4 %5\n%6")
                    .arg(config.name)
                    .arg(config.id)
                    .arg(pluginConfigPath)
                    .arg(command.program)
                    .arg(command.arguments.join(' '))
                    .arg(standardOutput.isEmpty() ? QString("<no stdout>") : standardOutput);
            }
            return true;
        }

        failureMessages.append(QString("%1 failed: %2")
            .arg(command.program)
            .arg(standardError.isEmpty() ? QString("<no stderr>") : standardError));
    }

    if (errorMessage) {
        *errorMessage = "Failed to run external solver. " + failureMessages.join(" | ");
    }
    return false;
}

bool ExternalProcessSolverPlugin::readResult(
    const QString &caseDirectory,
    QString *resultText,
    QString *errorMessage
) const
{
    const ExternalSolverPluginConfig config = ExternalSolverPluginConfigLoader::load(m_pluginDirectory);
    const QString resultFilePath = QDir(caseDirectory).filePath(config.outputFile);
    QFile file(resultFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "Failed to read external solver result: " + file.errorString();
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "Invalid external solver result: root is not a JSON object.";
        }
        return false;
    }

    const QJsonObject result = document.object();
    if (resultText) {
        *resultText = QString("%1: %2, iterations=%3, maxResidual=%4")
            .arg(result.value("solverName").toString(config.name))
            .arg(result.value("status").toString("unknown"))
            .arg(result.value("iterations").toInt())
            .arg(result.value("maxResidual").toDouble());
    }
    return true;
}
