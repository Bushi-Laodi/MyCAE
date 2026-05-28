#include "workflow/SolverCaseWorkflowController.h"

#include "project/ProjectModel.h"
#include "result/ResultObject.h"
#include "result/ResultManager.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "solver/SimulationCase.h"
#include "solver/SimulationCaseBuilder.h"
#include "solver/export/SolverCaseWriter.h"
#include "solver/plugin/SolverPlugin.h"
#include "solver/plugin/SolverPluginManager.h"
#include "workflow/ProjectWorkflowController.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QUuid>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

void appendMessages(SolverCaseWorkflowResult &result, const QStringList &messages)
{
    result.logMessages.append(messages);
}

void appendErrors(SolverCaseWorkflowResult &result, const QString &prefix, const QStringList &errors)
{
    for (const QString &error : errors) {
        result.logMessages.append(prefix + error);
    }
}

void appendWarnings(SolverCaseWorkflowResult &result, const QString &prefix, const QStringList &warnings)
{
    for (const QString &warning : warnings) {
        result.logMessages.append(prefix + warning);
    }
}

QString sanitizedDirectoryStem(QString value)
{
    value = value.trimmed();
    QString sanitized;
    sanitized.reserve(value.size());
    bool hasUsefulCharacter = false;

    for (const QChar ch : value) {
        if (ch.isLetterOrNumber()) {
            sanitized.append(ch);
            hasUsefulCharacter = true;
        } else if (ch.isSpace() || ch == '_' || ch == '-') {
            sanitized.append('_');
        }
    }

    return hasUsefulCharacter ? sanitized : QString();
}

QString simulationCaseStem(const SimulationCase &simulationCase)
{
    const QString nameStem = sanitizedDirectoryStem(simulationCase.name);
    if (!nameStem.isEmpty()) {
        return nameStem;
    }

    const QString idStem = sanitizedDirectoryStem(simulationCase.id);
    return idStem.isEmpty() ? QString("simulation_case") : idStem;
}

QString solverCaseDirectory(
    const ProjectModel &projectModel,
    const QString &pluginId,
    const SimulationCase &simulationCase
)
{
    const QString pluginStem = sanitizedDirectoryStem(pluginId);
    const QString safePluginId = pluginStem.isEmpty()
        ? QString("solver")
        : pluginStem;
    const QString runName = QString("%1_%2")
        .arg(simulationCaseStem(simulationCase))
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz"));
    return QDir(projectModel.project().rootPath).filePath("solver/" + safePluginId + "/" + runName);
}

QString resultId(const QString &pluginId)
{
    return pluginId + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

ResultObject makeResultObject(
    const QString &pluginId,
    const SolverPluginDescriptor &descriptor,
    const SimulationCase &simulationCase,
    const QString &caseDirectory,
    const SolverRunResult &runResult,
    const SolverResultReadResult &readResult
)
{
    ResultObject resultObject;
    resultObject.id = resultId(pluginId);
    resultObject.name = descriptor.name + " Result";
    resultObject.solverName = descriptor.name;
    resultObject.meshName = descriptor.id == "openfoam" && !simulationCase.cfdCase.meshName.trimmed().isEmpty()
        ? simulationCase.cfdCase.meshName
        : (descriptor.id == "calculix" && !simulationCase.structuralCase.meshName.trimmed().isEmpty()
            ? simulationCase.structuralCase.meshName
            : simulationCase.meshName);
    resultObject.casePath = caseDirectory;
    resultObject.logFile = runResult.logFile;
    resultObject.resultFiles = readResult.resultFiles;
    for (const QString &filePath : readResult.resultFiles) {
        const QString suffix = QFileInfo(filePath).suffix().toLower();
        if (suffix == "dat") {
            resultObject.datFile = filePath;
        } else if (suffix == "frd") {
            resultObject.frdFile = filePath;
        } else if (suffix == "sta") {
            resultObject.staFile = filePath;
        }
    }
    if (descriptor.id == "calculix") {
        resultObject.primaryFieldName = CalculiXResultFields::DisplacementMagnitude;
        resultObject.displayFieldName = resultObject.primaryFieldName;
        resultObject.deformationScale = 0.0;
        resultObject.availableFields = QStringList{
            CalculiXResultFields::Ux,
            CalculiXResultFields::Uy,
            CalculiXResultFields::Uz,
            CalculiXResultFields::DisplacementMagnitude,
            CalculiXResultFields::VonMisesStress
        };
    }
    resultObject.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    resultObject.success = true;
    resultObject.summary = readResult.summary;
    return resultObject;
}
}

SolverCaseWorkflowController::SolverCaseWorkflowController(
    ProjectModel &projectModel,
    const SolverPluginManager &solverPluginManager,
    ProjectWorkflowController &projectWorkflow
)
    : m_projectModel(projectModel)
    , m_solverPluginManager(solverPluginManager)
    , m_projectWorkflow(projectWorkflow)
{
}

SolverCaseWorkflowResult SolverCaseWorkflowController::runPlugin(const QString &pluginId) const
{
    SolverCaseWorkflowResult result;
    if (!m_projectModel.hasProject()) {
        result.logMessages.append(zh(u8"运行求解器失败：请先创建或打开工程。"));
        return result;
    }

    const ProjectWorkflowResult saveResult = m_projectWorkflow.saveSimulationCase();
    result.logMessages.append(saveResult.logMessages);
    if (!saveResult.success) {
        return result;
    }

    const SolverPlugin *plugin = m_solverPluginManager.pluginById(pluginId);
    if (!plugin) {
        result.logMessages.append(zh(u8"运行求解器失败：插件未注册：") + pluginId);
        return result;
    }

    const SolverPluginDescriptor *descriptor = m_solverPluginManager.descriptorById(pluginId);
    if (!descriptor) {
        result.logMessages.append(zh(u8"运行求解器失败：插件描述未注册：") + pluginId);
        return result;
    }

    const SimulationCase simulationCase = SimulationCaseBuilder::fromProjectModel(m_projectModel);
    const QString caseDirectory = solverCaseDirectory(m_projectModel, pluginId, simulationCase);
    if (!QDir().mkpath(caseDirectory)) {
        result.logMessages.append(zh(u8"运行求解器失败：无法创建算例目录：") + caseDirectory);
        return result;
    }

    result.logMessages.append(zh(u8"求解器插件：") + descriptor->name + " (" + descriptor->id + ")");
    result.logMessages.append(zh(u8"求解器类型：") + descriptor->solverFamily);
    result.logMessages.append(zh(u8"求解算例目录：") + caseDirectory);
    if (pluginId == "calculix") {
        result.logMessages.append(QString("Structural case data: %1 material(s), %2 constraint/load target(s), %3 load(s).")
            .arg(simulationCase.structuralCase.materials.size())
            .arg(simulationCase.structuralCase.constraints.size())
            .arg(simulationCase.structuralCase.loads.size()));
    } else if (pluginId == "openfoam") {
        result.logMessages.append(QString("CFD case data: %1 fluid material(s), %2 boundary condition(s), %3 field value(s).")
            .arg(simulationCase.cfdCase.materials.size())
            .arg(simulationCase.cfdCase.boundaries.size())
            .arg(simulationCase.cfdCase.fieldValues.size()));
    } else {
        result.logMessages.append(zh(u8"求解算例数据：%1 个材料，%2 个边界条件，%3 个载荷。")
            .arg(simulationCase.materials.size())
            .arg(simulationCase.boundaryConditions.size())
            .arg(simulationCase.loads.size()));
    }

    if (!descriptor->isUsable()) {
        result.logMessages.append(zh(u8"运行求解器失败：插件为预留状态或不可用。"));
        return result;
    }

    const SolverCaseContext context{&m_projectModel, &simulationCase, caseDirectory, pluginId};

    const SolverCaseWriter solverCaseWriter;
    const SolverCaseWriterResult exportResult = solverCaseWriter.writeCase(*plugin, context);
    appendMessages(result, exportResult.logMessages);
    appendWarnings(result, zh(u8"求解器导出警告："), exportResult.warnings);
    appendErrors(result, zh(u8"求解器导出失败："), exportResult.errors);
    if (!exportResult.success) {
        return result;
    }
    result.logMessages.append(zh(u8"求解器输入已导出。"));

    const SolverRunResult runResult = plugin->runCase(context);
    appendMessages(result, runResult.logMessages);
    appendErrors(result, zh(u8"求解器运行失败："), runResult.errors);
    if (!runResult.success) {
        return result;
    }

    const SolverResultReadResult readResult = plugin->readResult(context);
    appendMessages(result, readResult.logMessages);
    appendWarnings(result, zh(u8"求解结果警告："), readResult.warnings);
    appendErrors(result, zh(u8"求解结果读取失败："), readResult.errors);
    if (!readResult.success) {
        return result;
    }

    m_projectModel.resultRepository().results().push_back(
        makeResultObject(pluginId, *descriptor, simulationCase, caseDirectory, runResult, readResult)
    );
    QString saveError;
    if (!ResultManager().save(m_projectModel.project(), m_projectModel.resultRepository().results(), &saveError)) {
        result.logMessages.append(zh(u8"保存结果索引失败：") + saveError);
    } else {
        result.logMessages.append(zh(u8"结果索引已保存：") + ResultManager::relativeResultsFilePath());
    }

    result.logMessages.append(zh(u8"求解结果：") + readResult.summary);
    result.success = true;
    return result;
}
