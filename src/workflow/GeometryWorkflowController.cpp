#include "workflow/GeometryWorkflowController.h"

#include "geometry/GeometryBooleanController.h"
#include "geometry/GeometryManager.h"
#include "project/ProjectModel.h"
#include "workflow/ProjectWorkflowController.h"
#include "ui/PropertyPanel.h"
#include "ui/RenderView.h"
#include "workflow/SelectionController.h"

#include <QMessageBox>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}
}

GeometryWorkflowController::GeometryWorkflowController(
    GeometryManager &geometryManager,
    ProjectModel &projectModel,
    ProjectWorkflowController &projectWorkflow,
    PropertyPanel *propertyPanel,
    RenderView *renderView,
    QWidget *parent
)
    : m_geometryManager(geometryManager)
    , m_projectModel(projectModel)
    , m_projectWorkflow(projectWorkflow)
    , m_propertyPanel(propertyPanel)
    , m_renderView(renderView)
    , m_parent(parent)
{
}

GeometryWorkflowResult GeometryWorkflowController::createGeometry(GeometryCreateType type) const
{
    GeometryWorkflowResult workflowResult;
    const GeometryCreationController creationController(m_geometryManager);
    const GeometryCreationResult creationResult =
        creationController.createGeometry(m_parent, m_projectModel.project(), type);

    workflowResult.logMessages.append(creationResult.logMessages);
    if (creationResult.canceled) {
        workflowResult.canceled = true;
        return workflowResult;
    }
    if (!creationResult.success) {
        QMessageBox::warning(m_parent, zh(u8"创建几何失败"), creationResult.errorMessage);
        return workflowResult;
    }

    const QString createdGeometryName = creationResult.geometryObject.name;
    workflowResult.logMessages.append(m_projectWorkflow.loadGeometries().logMessages);
    m_projectWorkflow.refreshProjectTree();

    const SelectionController selectionController(m_projectModel, m_propertyPanel, m_renderView);
    const SelectionControllerResult selectionResult =
        selectionController.apply(Selection::item(SelectionKind::Geometry, createdGeometryName, createdGeometryName));
    workflowResult.logMessages.append(selectionResult.logMessages);
    if (!selectionResult.accepted) {
        workflowResult.logMessages.append(zh(u8"几何已保存，但无法选中：") + createdGeometryName);
    }

    workflowResult.logMessages.append(m_projectWorkflow.saveSimulationCase().logMessages);
    workflowResult.success = true;
    return workflowResult;
}

GeometryWorkflowResult GeometryWorkflowController::createBooleanGeometry() const
{
    GeometryWorkflowResult workflowResult;
    const GeometryBooleanController booleanController(m_geometryManager);
    const GeometryBooleanResult booleanResult = booleanController.createBooleanGeometry(m_parent, m_projectModel);

    workflowResult.logMessages.append(booleanResult.logMessages);
    if (booleanResult.canceled) {
        workflowResult.canceled = true;
        return workflowResult;
    }
    if (!booleanResult.success) {
        QMessageBox::warning(m_parent, zh(u8"布尔操作失败"), booleanResult.errorMessage);
        return workflowResult;
    }

    const QString createdGeometryName = booleanResult.geometryObject.name;
    workflowResult.logMessages.append(m_projectWorkflow.loadGeometries().logMessages);
    m_projectWorkflow.refreshProjectTree();

    const SelectionController selectionController(m_projectModel, m_propertyPanel, m_renderView);
    const SelectionControllerResult selectionResult =
        selectionController.apply(Selection::item(SelectionKind::Geometry, createdGeometryName, createdGeometryName));
    workflowResult.logMessages.append(selectionResult.logMessages);
    if (!selectionResult.accepted) {
        workflowResult.logMessages.append(zh(u8"布尔几何已保存，但无法选中：") + createdGeometryName);
    }

    workflowResult.logMessages.append(m_projectWorkflow.saveSimulationCase().logMessages);
    workflowResult.success = true;
    return workflowResult;
}
