#include "workflow/SelectionController.h"

#include "geometry/GeometryDisplayController.h"
#include "geometry/GeometryPropertyController.h"
#include "render/RenderHighlightController.h"
#include "result/ResultDisplayController.h"
#include "workflow/MeshWorkflowController.h"
#include "project/ProjectModel.h"
#include "result/ResultObject.h"
#include "solver/SimulationCaseBuilder.h"
#include "ui/PropertyPanel.h"
#include "ui/RenderView.h"
#include "ui/SolverDataController.h"

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

bool renderViewAlreadyShowsGeometry(RenderView *renderView, const QString &geometryName)
{
    return renderView && renderView->activeGeometryName() == geometryName;
}

QStringList displayFaceGroupGeometryIfNeeded(
    const ProjectModel &projectModel,
    RenderView *renderView,
    const FaceGroup &faceGroup
)
{
    QStringList logMessages;
    if (renderViewAlreadyShowsGeometry(renderView, faceGroup.geometryName)) {
        return logMessages;
    }

    if (const GeometryObject *geometry = projectModel.findGeometryByName(faceGroup.geometryName)) {
        const GeometryDisplayController displayController;
        const GeometryDisplayResult displayResult =
            displayController.displayGeometry(projectModel, *geometry, renderView);
        logMessages.append(displayResult.logMessages);
    } else {
        logMessages.append(zh(u8"面组所属几何尚未加载：") + faceGroup.geometryName);
    }
    return logMessages;
}
}

SelectionController::SelectionController(ProjectModel &projectModel, PropertyPanel *propertyPanel, RenderView *renderView)
    : m_projectModel(projectModel)
    , m_propertyPanel(propertyPanel)
    , m_renderView(renderView)
{
}

SelectionControllerResult SelectionController::apply(const Selection &selection) const
{
    switch (selection.kind) {
    case SelectionKind::Geometry:
        return showGeometry(selection.id);
    case SelectionKind::Mesh:
        return showMesh(selection.id);
    case SelectionKind::FaceGroup:
        return showFaceGroup(selection.id);
    case SelectionKind::MaterialCategory:
    case SelectionKind::BoundaryConditionCategory:
    case SelectionKind::LoadCategory:
    case SelectionKind::SolverCategory:
    case SelectionKind::ResultCategory:
        return showSolverCategory(selection.kind);
    case SelectionKind::Material: {
        SelectionControllerResult result;
        result.logMessages = SolverDataController::showMaterial(m_projectModel, m_propertyPanel, selection.id);
        result.accepted = m_projectModel.selection().kind == SelectionKind::Material;
        return result;
    }
    case SelectionKind::BoundaryCondition: {
        SelectionControllerResult result;
        result.logMessages =
            SolverDataController::showBoundaryCondition(m_projectModel, m_propertyPanel, selection.id);
        result.accepted = m_projectModel.selection().kind == SelectionKind::BoundaryCondition;
        if (const BoundaryCondition *boundaryCondition = m_projectModel.findBoundaryConditionById(selection.id)) {
            if (!boundaryCondition->target.faceGroupId.isEmpty()) {
                const FaceGroup *faceGroup = m_projectModel.findFaceGroupById(boundaryCondition->target.faceGroupId);
                if (faceGroup) {
                    result.logMessages.append(displayFaceGroupGeometryIfNeeded(m_projectModel, m_renderView, *faceGroup));

                    const RenderHighlightController highlightController;
                    const RenderHighlightResult highlightResult =
                        highlightController.highlightBoundaryConditionFaceGroup(*faceGroup, m_renderView);
                    result.logMessages.append(highlightResult.logMessages);
                } else {
                    result.logMessages.append(
                        zh(u8"边界条件引用的面组不存在：") + boundaryCondition->target.faceGroupId
                    );
                }
            }
        }
        return result;
    }
    case SelectionKind::Load: {
        SelectionControllerResult result;
        result.logMessages = SolverDataController::showLoad(m_projectModel, m_propertyPanel, selection.id);
        result.accepted = m_projectModel.selection().kind == SelectionKind::Load;
        if (const Load *load = m_projectModel.findLoadById(selection.id)) {
            if (const BoundaryCondition *boundaryCondition =
                    m_projectModel.findBoundaryConditionById(load->boundaryConditionId)) {
                if (const FaceGroup *faceGroup =
                        m_projectModel.findFaceGroupById(boundaryCondition->target.faceGroupId)) {
                    result.logMessages.append(displayFaceGroupGeometryIfNeeded(m_projectModel, m_renderView, *faceGroup));
                    const RenderHighlightController highlightController;
                    result.logMessages.append(
                        highlightController.highlightLoadFaceGroup(*faceGroup, m_renderView).logMessages
                    );
                }
            }
        }
        return result;
    }
    case SelectionKind::Result:
        return showResult(selection.id);
    case SelectionKind::None:
        m_projectModel.clearSelection();
        if (m_propertyPanel) {
            m_propertyPanel->showEmptySelection();
        }
        if (m_renderView) {
            m_renderView->showEmpty();
        }
        return {true, {}};
    }

    return {};
}

SelectionControllerResult SelectionController::showResult(const QString &resultId) const
{
    SelectionControllerResult result;
    ResultObject *resultObject = m_projectModel.findResultById(resultId);
    if (!resultObject) {
        result.logMessages.append(zh(u8"结果选择失败：未找到：") + resultId);
        return result;
    }

    m_projectModel.setSelection(Selection::item(SelectionKind::Result, resultObject->id, resultObject->name));
    if (m_propertyPanel) {
        m_propertyPanel->showResult(*resultObject);
    }

    const ResultDisplayController displayController;
    const ResultDisplayResult displayResult =
        displayController.displayResult(m_projectModel, *resultObject, m_renderView);
    result.logMessages.append(displayResult.logMessages);
    if (m_propertyPanel) {
        m_propertyPanel->showResult(*resultObject);
    }
    result.logMessages.append(zh(u8"结果已选择：") + resultObject->id);
    result.accepted = true;
    return result;
}

SelectionControllerResult SelectionController::showGeometry(const QString &geometryName) const
{
    SelectionControllerResult result;
    const GeometryObject *geometry = m_projectModel.findGeometryByName(geometryName);
    if (!geometry) {
        m_projectModel.clearSelectionIfKind(SelectionKind::Geometry);
        result.logMessages.append(zh(u8"几何选择失败：未找到：") + geometryName);
        return result;
    }

    m_projectModel.setSelection(Selection::item(SelectionKind::Geometry, geometry->name, geometry->name));

    const GeometryPropertyController propertyController;
    const GeometryPropertyResult propertyResult = propertyController.showGeometryProperties(
        m_projectModel,
        *geometry,
        m_propertyPanel
    );
    if (!propertyResult.success) {
        result.logMessages.append(propertyResult.errorMessage);
    }

    const GeometryDisplayController displayController;
    const GeometryDisplayResult displayResult = displayController.displayGeometry(
        m_projectModel,
        *geometry,
        m_renderView
    );
    result.logMessages.append(displayResult.logMessages);
    result.accepted = true;
    return result;
}

SelectionControllerResult SelectionController::showMesh(const QString &meshName) const
{
    SelectionControllerResult result;
    const MeshObject *meshObject = m_projectModel.findMeshByName(meshName);
    if (!meshObject) {
        m_projectModel.clearSelectionIfKind(SelectionKind::Mesh);
        result.logMessages.append(zh(u8"网格选择失败：未找到：") + meshName);
        return result;
    }

    m_projectModel.setSelection(Selection::item(SelectionKind::Mesh, meshObject->name, meshObject->name));
    if (m_propertyPanel) {
        m_propertyPanel->showMeshObject(*meshObject);
    }

    const MeshWorkflowController controller;
    const MeshWorkflowResult meshResult = controller.displayMeshObject(m_projectModel, *meshObject, m_renderView);
    result.logMessages.append(meshResult.logMessages);
    result.accepted = true;
    return result;
}

SelectionControllerResult SelectionController::showFaceGroup(const QString &faceGroupId) const
{
    SelectionControllerResult result;
    const FaceGroup *faceGroup = m_projectModel.findFaceGroupById(faceGroupId);
    if (!faceGroup) {
        result.logMessages.append(zh(u8"面组选择失败：未找到：") + faceGroupId);
        return result;
    }

    m_projectModel.setSelection(Selection::item(SelectionKind::FaceGroup, faceGroup->id, FaceGroups::displayName(*faceGroup)));
    result.logMessages.append(displayFaceGroupGeometryIfNeeded(m_projectModel, m_renderView, *faceGroup));

    const RenderHighlightController highlightController;
    const RenderHighlightResult highlightResult = highlightController.highlightFaceGroup(*faceGroup, m_renderView);
    result.logMessages.append(highlightResult.logMessages);

    if (m_propertyPanel) {
        m_propertyPanel->showFaceGroup(*faceGroup, m_projectModel.boundaryConditions(), m_projectModel.loads());
    }
    result.logMessages.append(zh(u8"面组已选择：") + faceGroup->id);
    result.accepted = true;
    return result;
}

SelectionControllerResult SelectionController::showSolverCategory(SelectionKind kind) const
{
    SelectionControllerResult result;
    m_projectModel.setSelection(Selection::category(kind));

    switch (kind) {
    case SelectionKind::MaterialCategory:
        result.logMessages = SolverDataController::showMaterialCategory(m_projectModel, m_propertyPanel);
        break;
    case SelectionKind::BoundaryConditionCategory:
        result.logMessages = SolverDataController::showBoundaryConditionCategory(m_projectModel, m_propertyPanel);
        break;
    case SelectionKind::LoadCategory:
        result.logMessages = SolverDataController::showLoadCategory(m_projectModel, m_propertyPanel);
        break;
    case SelectionKind::SolverCategory:
        if (m_propertyPanel) {
            m_propertyPanel->showSolverCategory(SimulationCaseBuilder::fromProjectModel(m_projectModel));
        }
        result.logMessages.append(zh(u8"求解器设置已显示。"));
        break;
    case SelectionKind::ResultCategory:
        if (m_propertyPanel) {
            m_propertyPanel->showResultCategory(m_projectModel.resultRepository().results());
        }
        result.logMessages.append(zh(u8"求解结果已显示。"));
        break;
    default:
        break;
    }

    result.accepted = true;
    return result;
}
