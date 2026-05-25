#include "GeometryBooleanController.h"

#include "geometry/GeometryBooleanOperation.h"
#include "geometry/GeometryManager.h"
#include "project/ProjectModel.h"
#include "ui/BooleanOperationDialog.h"

GeometryBooleanController::GeometryBooleanController(const GeometryManager &geometryManager)
    : m_geometryManager(geometryManager)
{
}

GeometryBooleanResult GeometryBooleanController::createBooleanGeometry(
    QWidget *parent,
    const ProjectModel &projectModel
) const
{
    GeometryBooleanResult result;
    if (!projectModel.hasProject()) {
        result.errorMessage = "Please create or open a project first.";
        result.logMessages.append("Boolean operation failed: no project is open.");
        return result;
    }
    const GeometryRepository &geometryRepository = projectModel.geometryRepository();
    if (geometryRepository.geometryObjects().size() < 2) {
        result.errorMessage = "Boolean operation requires at least two geometry objects.";
        result.logMessages.append("Boolean operation failed: less than two geometry objects are available.");
        return result;
    }

    const QString initialGeometryName = projectModel.selection().kind == SelectionKind::Geometry
        ? projectModel.selection().id
        : QString();
    const std::optional<BooleanOperationDialogResult> dialogResult =
        BooleanOperationDialog::getOperation(parent, geometryRepository.geometryObjects(), initialGeometryName);
    if (!dialogResult) {
        result.canceled = true;
        result.logMessages.append("Boolean operation canceled.");
        return result;
    }

    const GeometryObject *leftGeometry = geometryRepository.findGeometryByName(dialogResult->leftGeometryName);
    const GeometryObject *rightGeometry = geometryRepository.findGeometryByName(dialogResult->rightGeometryName);
    if (!leftGeometry || !rightGeometry) {
        result.errorMessage = "Selected boolean input geometry was not found.";
        result.logMessages.append("Boolean operation failed: input geometry was not found.");
        return result;
    }

    QString errorMessage;
    GeometryObject createdGeometry;
    if (!m_geometryManager.createBooleanGeometry(
            projectModel.project(),
            *leftGeometry,
            *rightGeometry,
            dialogResult->operationType,
            dialogResult->resultName,
            &createdGeometry,
            &errorMessage)) {
        result.errorMessage = errorMessage;
        result.logMessages.append("Boolean operation failed: " + errorMessage);
        return result;
    }

    result.success = true;
    result.geometryObject = createdGeometry;
    result.logMessages.append(QString("%1 geometry created: %2")
        .arg(toDisplayString(dialogResult->operationType), createdGeometry.name));
    result.logMessages.append("BREP saved: " + createdGeometry.brepFile);
    result.logMessages.append("STEP saved: " + createdGeometry.stepFile);
    return result;
}
