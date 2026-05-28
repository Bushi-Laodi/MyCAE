#include "ui/MainWindowStateController.h"

#include "diagnostics/DiagnosticCollector.h"
#include "picking/PickController.h"
#include "picking/PickMode.h"
#include "project/ProjectModel.h"
#include "project/SelectionState.h"
#include "result/ResultObject.h"
#include "ui/MainWindowActions.h"
#include "ui/RenderView.h"
#include "ui/ResultPostprocessPanel.h"

#include <QAction>
#include <QString>

void MainWindowStateController::update(
    const MainWindowActions &actions,
    const ProjectModel &projectModel,
    const PickController &pickController,
    const DiagnosticCollector &diagnosticCollector,
    const RenderView *renderView,
    ResultPostprocessPanel *resultPostprocessPanel
)
{
    const bool hasProject = projectModel.hasProject();
    const SelectionCapabilities capabilities = projectModel.selectionCapabilities();
    const bool hasPickedFaces = pickController.hasSelection();
    const bool hasSelectedFaceGroup = projectModel.selection().kind == SelectionKind::FaceGroup;
    const ResultObject *selectedResult = projectModel.resultForSelection();

    if (actions.createBox) {
        actions.createBox->setEnabled(hasProject);
    }
    if (actions.createCylinder) {
        actions.createCylinder->setEnabled(hasProject);
    }
    if (actions.createSphere) {
        actions.createSphere->setEnabled(hasProject);
    }
    if (actions.importStep) {
        actions.importStep->setEnabled(hasProject);
    }
    if (actions.createBoolean) {
        actions.createBoolean->setEnabled(hasProject && projectModel.geometryRepository().geometryObjects().size() >= 2);
    }
    if (actions.deleteGeometry) {
        actions.deleteGeometry->setEnabled(hasProject && projectModel.selection().kind == SelectionKind::Geometry);
    }
    if (actions.showGeometryEdges) {
        actions.showGeometryEdges->setEnabled(hasProject);
        actions.showGeometryEdges->setChecked(renderView && renderView->geometryEdgesVisible());
    }
    if (actions.showOrientationMarker) {
        actions.showOrientationMarker->setEnabled(renderView != nullptr);
        actions.showOrientationMarker->setChecked(renderView && renderView->orientationMarkerVisible());
    }
    if (actions.generateMesh) {
        actions.generateMesh->setEnabled(hasProject && capabilities.canGenerateMesh);
    }
    if (actions.readMeshInfo) {
        actions.readMeshInfo->setEnabled(hasProject && capabilities.canReadMeshInfo);
    }
    if (actions.showMesh) {
        actions.showMesh->setEnabled(hasProject && capabilities.canShowMesh);
    }
    if (actions.pickFace) {
        actions.pickFace->setEnabled(hasProject);
        actions.pickFace->setChecked(pickController.mode() == PickMode::Face);
    }
    if (actions.clearPick) {
        actions.clearPick->setEnabled(hasProject);
    }
    if (actions.createFaceGroupFromPick) {
        actions.createFaceGroupFromPick->setEnabled(hasProject && hasPickedFaces);
    }
    if (actions.addPickedFacesToFaceGroup) {
        actions.addPickedFacesToFaceGroup->setEnabled(hasProject && hasPickedFaces && hasSelectedFaceGroup);
    }
    if (actions.removePickedFacesFromFaceGroup) {
        actions.removePickedFacesFromFaceGroup->setEnabled(hasProject && hasPickedFaces && hasSelectedFaceGroup);
    }
    if (actions.clearFaceGroupFaces) {
        actions.clearFaceGroupFaces->setEnabled(hasProject && hasSelectedFaceGroup);
    }
    if (actions.renameFaceGroup) {
        actions.renameFaceGroup->setEnabled(hasProject && hasSelectedFaceGroup);
    }
    if (actions.deleteFaceGroup) {
        actions.deleteFaceGroup->setEnabled(hasProject && hasSelectedFaceGroup);
    }
    if (actions.setFaceGroupLocalMeshSize) {
        actions.setFaceGroupLocalMeshSize->setEnabled(hasProject && hasSelectedFaceGroup);
    }
    if (actions.toggleFaceGroupPhysicalGroup) {
        actions.toggleFaceGroupPhysicalGroup->setEnabled(hasProject && hasSelectedFaceGroup);
    }
    if (actions.createMaterial) {
        actions.createMaterial->setEnabled(hasProject);
    }
    if (actions.createStructuralMaterial) {
        actions.createStructuralMaterial->setEnabled(hasProject);
    }
    if (actions.createFluidMaterial) {
        actions.createFluidMaterial->setEnabled(hasProject);
    }
    if (actions.createBoundaryCondition) {
        actions.createBoundaryCondition->setEnabled(hasProject);
    }
    if (actions.createStructuralBoundaryCondition) {
        actions.createStructuralBoundaryCondition->setEnabled(hasProject);
    }
    if (actions.createCfdBoundaryCondition) {
        actions.createCfdBoundaryCondition->setEnabled(hasProject);
    }
    if (actions.createLoad) {
        actions.createLoad->setEnabled(hasProject);
    }
    if (actions.createStructuralLoad) {
        actions.createStructuralLoad->setEnabled(hasProject);
    }
    if (actions.createCfdFieldValue) {
        actions.createCfdFieldValue->setEnabled(hasProject);
    }
    if (actions.editSolverData) {
        actions.editSolverData->setEnabled(hasProject && capabilities.canEditSolverData);
    }
    if (actions.deleteSolverData) {
        actions.deleteSolverData->setEnabled(hasProject && capabilities.canDeleteSolverData);
    }
    for (QAction *runSolverAction : actions.runSolvers) {
        if (runSolverAction) {
            runSolverAction->setEnabled(hasProject && runSolverAction->property("solverUsable").toBool());
        }
    }
    for (QAction *fieldAction : actions.resultFields) {
        if (!fieldAction) {
            continue;
        }
        fieldAction->setEnabled(hasProject && selectedResult);
        if (selectedResult) {
            const QString field = fieldAction->data().toString();
            fieldAction->setChecked(field == selectedResult->displayFieldName
                || (selectedResult->displayFieldName.isEmpty() && field == selectedResult->primaryFieldName));
        }
    }
    for (QAction *scaleAction : actions.resultScales) {
        if (!scaleAction) {
            continue;
        }
        scaleAction->setEnabled(hasProject && selectedResult);
        if (selectedResult) {
            scaleAction->setChecked(scaleAction->data().toDouble() == selectedResult->deformationScale);
        }
    }
    if (actions.exportScreenshot) {
        actions.exportScreenshot->setEnabled(hasProject);
    }
    if (actions.projectResources) {
        actions.projectResources->setEnabled(hasProject);
    }
    if (actions.clearDiagnostics) {
        actions.clearDiagnostics->setEnabled(!diagnosticCollector.diagnostics().isEmpty());
    }
    if (resultPostprocessPanel) {
        resultPostprocessPanel->setEnabledForResult(hasProject && selectedResult);
    }
}
