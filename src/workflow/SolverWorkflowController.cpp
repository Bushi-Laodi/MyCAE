#include "workflow/SolverWorkflowController.h"

#include "project/ProjectModel.h"
#include "solver/plugin/SolverPluginManager.h"
#include "workflow/SolverCaseWorkflowController.h"
#include "workflow/ProjectWorkflowController.h"
#include "ui/PropertyPanel.h"
#include "ui/RenderView.h"
#include "workflow/SelectionController.h"
#include "ui/SolverDataController.h"

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
    result.success = caseResult.success;
    result.logMessages = caseResult.logMessages;
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
