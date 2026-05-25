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
    resultObject.meshName = simulationCase.meshName;
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
        result.logMessages.append("Run solver failed: create or open a project first.");
        return result;
    }

    const ProjectWorkflowResult saveResult = m_projectWorkflow.saveSimulationCase();
    result.logMessages.append(saveResult.logMessages);
    if (!saveResult.success) {
        return result;
    }

    const SolverPlugin *plugin = m_solverPluginManager.pluginById(pluginId);
    if (!plugin) {
        result.logMessages.append("Run solver failed: plugin is not registered: " + pluginId);
        return result;
    }

    const SolverPluginDescriptor *descriptor = m_solverPluginManager.descriptorById(pluginId);
    if (!descriptor) {
        result.logMessages.append("Run solver failed: plugin descriptor is not registered: " + pluginId);
        return result;
    }

    const SimulationCase simulationCase = SimulationCaseBuilder::fromProjectModel(m_projectModel);
    const QString caseDirectory = solverCaseDirectory(m_projectModel, pluginId, simulationCase);
    if (!QDir().mkpath(caseDirectory)) {
        result.logMessages.append("Run solver failed: cannot create case directory: " + caseDirectory);
        return result;
    }

    result.logMessages.append("Solver plugin: " + descriptor->name + " (" + descriptor->id + ")");
    result.logMessages.append("Solver family: " + descriptor->solverFamily);
    result.logMessages.append("Solver case directory: " + caseDirectory);
    result.logMessages.append(QString("Solver case data: %1 materials, %2 boundary conditions, %3 loads.")
        .arg(simulationCase.materials.size())
        .arg(simulationCase.boundaryConditions.size())
        .arg(simulationCase.loads.size()));

    if (!descriptor->isUsable()) {
        result.logMessages.append("Run solver failed: plugin is reserved or unavailable.");
        return result;
    }

    const SolverCaseContext context{&m_projectModel, &simulationCase, caseDirectory, pluginId};

    const SolverCaseWriter solverCaseWriter;
    const SolverCaseWriterResult exportResult = solverCaseWriter.writeCase(*plugin, context);
    appendMessages(result, exportResult.logMessages);
    appendWarnings(result, "Solver export warning: ", exportResult.warnings);
    appendErrors(result, "Solver export failed: ", exportResult.errors);
    if (!exportResult.success) {
        return result;
    }
    result.logMessages.append("Solver input exported.");

    const SolverRunResult runResult = plugin->runCase(context);
    appendMessages(result, runResult.logMessages);
    appendErrors(result, "Solver run failed: ", runResult.errors);
    if (!runResult.success) {
        return result;
    }

    const SolverResultReadResult readResult = plugin->readResult(context);
    appendMessages(result, readResult.logMessages);
    appendWarnings(result, "Solver result warning: ", readResult.warnings);
    appendErrors(result, "Solver result read failed: ", readResult.errors);
    if (!readResult.success) {
        return result;
    }

    m_projectModel.resultRepository().results().push_back(
        makeResultObject(pluginId, *descriptor, simulationCase, caseDirectory, runResult, readResult)
    );
    QString saveError;
    if (!ResultManager().save(m_projectModel.project(), m_projectModel.resultRepository().results(), &saveError)) {
        result.logMessages.append("Save result index failed: " + saveError);
    } else {
        result.logMessages.append("Result index saved: " + ResultManager::relativeResultsFilePath());
    }

    result.logMessages.append("Solver result: " + readResult.summary);
    result.success = true;
    return result;
}
