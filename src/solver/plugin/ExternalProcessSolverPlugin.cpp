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

SolverPluginDescriptor ExternalProcessSolverPlugin::descriptor() const
{
    const ExternalSolverPluginConfig config = ExternalSolverPluginConfigLoader::load(m_pluginDirectory);
    SolverPluginDescriptor descriptor;
    descriptor.id = config.id;
    descriptor.name = config.name;
    descriptor.vendor = "External";
    descriptor.solverFamily = config.type;
    descriptor.description = config.description;
    descriptor.status = SolverPluginStatus::Ready;
    descriptor.capabilities.canExportCase = true;
    descriptor.capabilities.canRunCase = true;
    descriptor.capabilities.canReadResult = true;
    descriptor.capabilities.analysisTypes = config.supportedAnalysisTypes;
    descriptor.capabilities.inputFormats = {config.inputFile};
    descriptor.capabilities.outputFormats = {config.outputFile};
    return descriptor;
}

SolverCaseWriterResult ExternalProcessSolverPlugin::exportCase(const SolverCaseContext &context) const
{
    SolverCaseWriterResult result;
    if (!context.isValid()) {
        result.errors.append("External solver export failed: invalid solver case context.");
        return result;
    }

    const ExternalSolverPluginConfig config = ExternalSolverPluginConfigLoader::load(m_pluginDirectory);
    const QString caseFilePath = QDir(context.caseDirectory).filePath(config.inputFile);
    QString errorMessage;
    if (!SimulationCaseJsonWriter::writeCaseFile(*context.simulationCase, caseFilePath, &errorMessage)) {
        result.errors.append(errorMessage);
        return result;
    }

    result.success = true;
    result.caseRootPath = context.caseDirectory;
    result.writtenFiles.append(caseFilePath);
    result.logMessages.append("External solver input exported: " + caseFilePath);
    return result;
}

SolverRunResult ExternalProcessSolverPlugin::runCase(const SolverCaseContext &context) const
{
    SolverRunResult result;
    if (!context.isValid()) {
        result.errors.append("External solver run failed: invalid solver case context.");
        return result;
    }

    const ExternalSolverPluginConfig config = ExternalSolverPluginConfigLoader::load(m_pluginDirectory);
    const QString caseFilePath = QDir(context.caseDirectory).filePath(config.inputFile);
    if (!QFileInfo::exists(caseFilePath)) {
        result.errors.append("External solver input does not exist: " + caseFilePath);
        return result;
    }

    const QString pluginConfigPath = ExternalSolverPluginConfigLoader::configPath(m_pluginDirectory);
    if (!QFileInfo::exists(pluginConfigPath)) {
        result.errors.append("External solver plugin description does not exist: " + pluginConfigPath);
        return result;
    }

    if (!config.script.isEmpty()
            && !QFileInfo::exists(ExternalSolverPluginConfigLoader::scriptPath(m_pluginDirectory, config))) {
        result.errors.append(
            "External solver script does not exist: "
            + ExternalSolverPluginConfigLoader::scriptPath(m_pluginDirectory, config)
        );
        return result;
    }

    const QString resultFilePath = QDir(context.caseDirectory).filePath(config.outputFile);
    const std::vector<ProcessCommand> commands = commandsForConfig(config, m_pluginDirectory, caseFilePath, resultFilePath);
    if (commands.empty()) {
        result.errors.append("External solver plugin has no runnable command: " + pluginConfigPath);
        return result;
    }

    QStringList failureMessages;
    for (const ProcessCommand &command : commands) {
        QString standardOutput;
        QString standardError;
        if (runProcess(command.program, command.arguments, context.caseDirectory, &standardOutput, &standardError)) {
            result.success = true;
            result.exitCode = 0;
            result.command = command.program + " " + command.arguments.join(' ');
            result.workingDirectory = context.caseDirectory;
            result.standardOutput = standardOutput;
            result.standardError = standardError;
            result.logMessages.append(QString("External solver plugin: %1 (%2)").arg(config.name, config.id));
            result.logMessages.append("Config: " + pluginConfigPath);
            result.logMessages.append("Command: " + result.command);
            result.logMessages.append(standardOutput.isEmpty() ? QString("<no stdout>") : standardOutput);
            return result;
        }

        failureMessages.append(QString("%1 failed: %2")
            .arg(command.program)
            .arg(standardError.isEmpty() ? QString("<no stderr>") : standardError));
    }

    result.errors.append("Failed to run external solver. " + failureMessages.join(" | "));
    return result;
}

SolverResultReadResult ExternalProcessSolverPlugin::readResult(const SolverCaseContext &context) const
{
    SolverResultReadResult result;
    if (!context.isValid()) {
        result.errors.append("External solver result read failed: invalid solver case context.");
        return result;
    }

    const ExternalSolverPluginConfig config = ExternalSolverPluginConfigLoader::load(m_pluginDirectory);
    const QString resultFilePath = QDir(context.caseDirectory).filePath(config.outputFile);
    QFile file(resultFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.errors.append("Failed to read external solver result: " + file.errorString());
        return result;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        result.errors.append("Invalid external solver result: root is not a JSON object.");
        return result;
    }

    const QJsonObject resultObject = document.object();
    result.success = true;
    result.resultFiles.append(resultFilePath);
    result.summary = QString("%1: %2, iterations=%3, maxResidual=%4")
        .arg(resultObject.value("solverName").toString(config.name))
        .arg(resultObject.value("status").toString("unknown"))
        .arg(resultObject.value("iterations").toInt())
        .arg(resultObject.value("maxResidual").toDouble());
    result.logMessages.append("External solver result read: " + resultFilePath);
    return result;
}
