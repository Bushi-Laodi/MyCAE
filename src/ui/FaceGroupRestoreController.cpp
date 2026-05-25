#include "ui/FaceGroupRestoreController.h"

#include "geometry/GeometryManager.h"
#include "project/ProjectModel.h"
#include "project/ProjectManager.h"
#include "project/SelectionState.h"
#include "ui/MainWindowDocks.h"
#include "ui/PropertyPanel.h"
#include "workflow/ProjectWorkflowController.h"

FaceGroupRestoreController::FaceGroupRestoreController(
    FaceGroupRestoreContext context,
    FaceGroupRestoreCallbacks callbacks
)
    : m_context(context)
    , m_callbacks(std::move(callbacks))
{
}

void FaceGroupRestoreController::restoreAfterUndoStackChange(const QString &selectionId) const
{
    ProjectWorkflowController projectWorkflow(
        m_context.projectManager,
        m_context.geometryManager,
        m_context.projectModel,
        m_context.docks.projectTreePanel,
        m_context.docks.propertyPanel,
        m_context.docks.renderView,
        m_context.window
    );
    projectWorkflow.refreshFaceGroupTree();
    if (m_callbacks.writeLogMessages) {
        m_callbacks.writeLogMessages(projectWorkflow.saveSimulationCase().logMessages);
    }

    if (!selectionId.isEmpty() && m_context.projectModel.findFaceGroupById(selectionId)) {
        if (m_callbacks.applySelection) {
            m_callbacks.applySelection(Selection::item(SelectionKind::FaceGroup, selectionId));
        }
    } else {
        m_context.projectModel.clearSelectionIfKind(SelectionKind::FaceGroup);
        if (m_context.docks.propertyPanel) {
            m_context.docks.propertyPanel->showEmptySelection();
        }
    }
    if (m_callbacks.updateActionStates) {
        m_callbacks.updateActionStates();
    }
}
