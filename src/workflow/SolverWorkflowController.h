#pragma once

#include "solver/SolverDataService.h"

#include <QString>
#include <QStringList>

class ProjectModel;
class ProjectWorkflowController;
class PropertyPanel;
class RenderView;
class SolverPluginManager;
class QWidget;

struct SolverWorkflowResult
{
    bool success = false;
    QStringList logMessages;
};

class SolverWorkflowController
{
public:
    SolverWorkflowController(
        ProjectModel &projectModel,
        const SolverPluginManager &solverPluginManager,
        ProjectWorkflowController &projectWorkflow,
        PropertyPanel *propertyPanel,
        RenderView *renderView,
        QWidget *parent
    );

    SolverWorkflowResult createMaterial() const;
    SolverWorkflowResult createStructuralMaterial() const;
    SolverWorkflowResult createFluidMaterial() const;
    SolverWorkflowResult createSectionAssignment() const;
    SolverWorkflowResult createBoundaryCondition() const;
    SolverWorkflowResult createStructuralBoundaryCondition() const;
    SolverWorkflowResult createCfdBoundaryCondition() const;
    SolverWorkflowResult createLoad() const;
    SolverWorkflowResult createStructuralLoad() const;
    SolverWorkflowResult createCfdFieldValue() const;
    SolverWorkflowResult editSelectedData() const;
    SolverWorkflowResult deleteSelectedData() const;
    SolverWorkflowResult runSolverPlugin(const QString &pluginId) const;

private:
    SolverWorkflowResult handleSolverDataResult(const SolverDataServiceResult &result) const;

    ProjectModel &m_projectModel;
    const SolverPluginManager &m_solverPluginManager;
    ProjectWorkflowController &m_projectWorkflow;
    PropertyPanel *m_propertyPanel = nullptr;
    RenderView *m_renderView = nullptr;
    QWidget *m_parent = nullptr;
};
