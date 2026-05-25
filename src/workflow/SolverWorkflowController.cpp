#include "workflow/SolverWorkflowController.h"

#include "project/ProjectModel.h"
#include "result/ResultObject.h"
#include "solver/SimulationCaseBuilder.h"
#include "solver/plugin/SolverPlugin.h"
#include "solver/plugin/SolverPluginManager.h"
#include "workflow/SolverCaseWorkflowController.h"
#include "workflow/ProjectWorkflowController.h"
#include "ui/PropertyPanel.h"
#include "ui/RenderView.h"
#include "workflow/SelectionController.h"
#include "ui/SolverDataController.h"

#include <QDir>
#include <QDateTime>

SolverWorkflowController::SolverWorkflowController(
    ProjectModel &projectModel,
    const SolverPluginManager &solverPluginManager,
    ProjectWorkflowController &projectWorkflow,
    PropertyPanel *propertyPanel,
    RenderView *renderView,
    QWidget *parent
)
    : m_projectModel(projectModel)
    , m_solverPluginManager(solverPluginManager)
    , m_projectWorkflow(projectWorkflow)
    , m_propertyPanel(propertyPanel)
    , m_renderView(renderView)
    , m_parent(parent)
{
}

SolverWorkflowResult SolverWorkflowController::createMaterial() const
{
    return handleSolverDataResult(SolverDataController::createMaterial(m_parent, m_projectModel));
}

SolverWorkflowResult SolverWorkflowController::createBoundaryCondition() const
{
    return handleSolverDataResult(SolverDataController::createBoundaryCondition(m_parent, m_projectModel));
}

SolverWorkflowResult SolverWorkflowController::createLoad() const
{
    return handleSolverDataResult(SolverDataController::createLoad(m_parent, m_projectModel));
}

SolverWorkflowResult SolverWorkflowController::editSelectedData() const
{
    return handleSolverDataResult(SolverDataController::editSelected(m_parent, m_projectModel));
}

SolverWorkflowResult SolverWorkflowController::deleteSelectedData() const
{
    return handleSolverDataResult(SolverDataController::deleteSelected(m_parent, m_projectModel));
}

SolverWorkflowResult SolverWorkflowController::runSolverPlugin(const QString &pluginId) const
{
    const SolverCaseWorkflowController solverCaseWorkflow(
        m_projectModel,
        m_solverPluginManager,
        m_projectWorkflow
    );
    const SolverCaseWorkflowResult caseResult = solverCaseWorkflow.runPlugin(pluginId);

    SolverWorkflowResult result;
    if (!m_projectModel.hasProject()) {
        result.logMessages.append("Run solver failed: create or open a project first.");
        return result;
    }

    const ProjectWorkflowResult saveResult = m_projectWorkflow.saveSimulationCase();
    result.logMessages.append(saveResult.logMessages);
    if (!saveResult.success) {
        return result;
    }

    const SimulationCase simulationCase = SimulationCaseBuilder::fromProjectModel(m_projectModel);
    const QString caseDirectory = QDir(m_projectModel.project().rootPath).filePath("solver/" + pluginId);

    const SolverPlugin *plugin = m_solverPluginManager.pluginById(pluginId);
    if (!plugin) {
        result.logMessages.append("Run solver failed: plugin is not registered: " + pluginId);
        return result;
    }

    QString errorMessage;
    result.logMessages.append("Solver plugin: " + plugin->name());
    result.logMessages.append("Solver case directory: " + caseDirectory);
    result.logMessages.append(QString("Solver case data: %1 materials, %2 boundary conditions, %3 loads.")
        .arg(simulationCase.materials.size())
        .arg(simulationCase.boundaryConditions.size())
        .arg(simulationCase.loads.size()));

    if (!plugin->exportCase(simulationCase, caseDirectory, &errorMessage)) {
        result.logMessages.append("Solver export failed: " + errorMessage);
        return result;
    }
    result.logMessages.append("Solver input exported.");

    QString solverLog;
    if (!plugin->runCase(caseDirectory, &solverLog, &errorMessage)) {
        result.logMessages.append("Solver run failed: " + errorMessage);
        return result;
    }
    if (!solverLog.isEmpty()) {
        result.logMessages.append(solverLog);
    }

    QString resultText;
    if (!plugin->readResult(caseDirectory, &resultText, &errorMessage)) {
        result.logMessages.append("Solver result read failed: " + errorMessage);
        return result;
    }

    result.logMessages.append("Solver result: " + resultText);

    ResultObject resultObject;
    resultObject.id = plugin->id() + "_" + QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmsszzz");
    resultObject.name = plugin->name() + " Result";
    resultObject.solverName = plugin->name();
    resultObject.casePath = caseDirectory;
    resultObject.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    resultObject.success = true;
    resultObject.summary = resultText;
    m_projectModel.resultRepository().results().push_back(resultObject);
    m_projectWorkflow.refreshResultTree();
    result.logMessages.append("Result object created: " + resultObject.id);

    result.success = true;
    return result;
}

SolverWorkflowResult SolverWorkflowController::handleSolverDataResult(const SolverDataServiceResult &dataResult) const
{
    SolverWorkflowResult result;
    result.logMessages.append(dataResult.logMessages);

    if (!dataResult.changed) {
        return result;
    }

    result.logMessages.append(m_projectWorkflow.saveSimulationCase().logMessages);
    m_projectWorkflow.refreshSolverDataTree();

    Selection selection;
    switch (dataResult.selectionKind) {
    case SolverDataSelectionKind::MaterialCategory:
        selection = Selection::category(SelectionKind::MaterialCategory);
        break;
    case SolverDataSelectionKind::BoundaryConditionCategory:
        selection = Selection::category(SelectionKind::BoundaryConditionCategory);
        break;
    case SolverDataSelectionKind::LoadCategory:
        selection = Selection::category(SelectionKind::LoadCategory);
        break;
    case SolverDataSelectionKind::Material:
        selection = Selection::item(SelectionKind::Material, dataResult.selectionId);
        break;
    case SolverDataSelectionKind::BoundaryCondition:
        selection = Selection::item(SelectionKind::BoundaryCondition, dataResult.selectionId);
        break;
    case SolverDataSelectionKind::Load:
        selection = Selection::item(SelectionKind::Load, dataResult.selectionId);
        break;
    case SolverDataSelectionKind::None:
        break;
    }

    if (!selection.isNone()) {
        const SelectionController selectionController(m_projectModel, m_propertyPanel, m_renderView);
        result.logMessages.append(selectionController.apply(selection).logMessages);
    }

    result.success = true;
    return result;
}
