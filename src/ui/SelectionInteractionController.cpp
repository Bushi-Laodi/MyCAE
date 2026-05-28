#include "ui/SelectionInteractionController.h"

#include "geometry/FaceGroup.h"
#include "picking/PickController.h"
#include "project/ProjectModel.h"
#include "project/SelectionState.h"
#include "result/ResultAnimationController.h"
#include "result/ResultObject.h"
#include "ui/MainWindowDocks.h"
#include "ui/PropertyPanel.h"
#include "ui/ResultPostprocessPanel.h"
#include "workflow/SelectionController.h"

namespace
{
bool selectionHasReferenceHighlight(SelectionKind kind)
{
    return kind == SelectionKind::FaceGroup
        || kind == SelectionKind::BoundaryCondition
        || kind == SelectionKind::Load;
}
}

SelectionInteractionController::SelectionInteractionController(
    SelectionInteractionContext context,
    SelectionInteractionCallbacks callbacks
)
    : m_context(context)
    , m_callbacks(std::move(callbacks))
{
}

void SelectionInteractionController::applySelection(const Selection &selection) const
{
    const ResultObject *previousResult = m_context.projectModel.resultForSelection();
    if (m_context.resultAnimationController.isRunning()
            && (selection.kind != SelectionKind::Result
                || !previousResult
                || selection.id != previousResult->id)) {
        if (m_callbacks.stopResultAnimation) {
            m_callbacks.stopResultAnimation();
        }
    }

    const SelectionController controller(
        m_context.projectModel,
        m_context.docks.propertyPanel,
        m_context.docks.renderView
    );
    if (selection.kind == SelectionKind::Geometry) {
        m_context.pickController.setTargetGeometry(selection.id);
    } else if (selection.kind == SelectionKind::FaceGroup) {
        if (const FaceGroup *faceGroup = m_context.projectModel.findFaceGroupById(selection.id)) {
            m_context.pickController.setTargetGeometry(faceGroup->geometryName);
        }
    }

    const SelectionControllerResult result = controller.apply(selection);
    if (m_callbacks.writeLogMessages) {
        m_callbacks.writeLogMessages(result.logMessages);
    }
    if (m_context.docks.resultPostprocessPanel) {
        m_context.docks.resultPostprocessPanel->setResult(m_context.projectModel.resultForSelection());
    }
    if (selection.kind != SelectionKind::FaceGroup) {
        m_context.pickController.clear(
            m_context.docks.renderView,
            !selectionHasReferenceHighlight(selection.kind)
        );
    }
    if (m_callbacks.updateActionStates) {
        m_callbacks.updateActionStates();
    }
}

void SelectionInteractionController::handleFacePicked(const PickSelection &selection) const
{
    const PickControllerResult result = m_context.pickController.acceptSelection(selection, m_context.docks.renderView);
    if (m_callbacks.writeLogMessages) {
        m_callbacks.writeLogMessages(result.logMessages);
    }
    if (m_context.docks.propertyPanel && result.success) {
        m_context.docks.propertyPanel->showPickState(
            m_context.pickController.mode(),
            m_context.pickController.geometryName(),
            m_context.pickController.selectedFaceIndices()
        );
    }
    if (result.success && m_callbacks.showStatusMessage) {
        m_callbacks.showStatusMessage(
            QString("Pick: %1 face(s) on %2")
                .arg(result.selectedFaceCount)
                .arg(result.geometryName)
        );
    }
    if (m_callbacks.updateActionStates) {
        m_callbacks.updateActionStates();
    }
}
