#include "result/ResultDisplayController.h"

#include "mesh/MeshToVtkConverter.h"
#include "result/ResultDataLoader.h"
#include "result/ResultExtremaCalculator.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "ui/RenderView.h"

#include <QFileInfo>
#include <vtkUnstructuredGrid.h>

namespace
{
QString scalarUnit(const QString &fieldName)
{
    if (fieldName == CalculiXResultFields::VonMisesStress) {
        return "Pa";
    }
    return "model length";
}

bool fileExistsOrEmpty(const QString &filePath)
{
    return !filePath.isEmpty() && QFileInfo::exists(filePath);
}

QStringList fileCompletenessMessages(const ResultObject &resultObject)
{
    QStringList messages;
    if (!fileExistsOrEmpty(resultObject.datFile)) {
        messages.append("Missing .dat file.");
    }
    if (!fileExistsOrEmpty(resultObject.staFile)) {
        messages.append("Missing .sta file.");
    }
    if (!fileExistsOrEmpty(resultObject.logFile)) {
        messages.append("Missing .log file.");
    }
    if (!fileExistsOrEmpty(resultObject.frdFile)) {
        messages.append("Missing .frd file.");
    }
    return messages;
}
}

ResultDisplayResult ResultDisplayController::displayResult(
    const ProjectModel &projectModel,
    ResultObject &resultObject,
    RenderView *renderView,
    bool resetCamera
) const
{
    ResultDisplayResult displayResult;
    resultObject.checkMessages = fileCompletenessMessages(resultObject);
    resultObject.resultFilesComplete = resultObject.checkMessages.isEmpty();
    if (!renderView) {
        displayResult.logMessages.append("Result display skipped: render view is not available.");
        return displayResult;
    }
    const ResultDataLoadResult loaded = ResultDataLoader().loadCalculiXResult(projectModel, resultObject);
    resultObject.checkMessages.append(loaded.warnings);
    displayResult.logMessages.append(loaded.warnings);
    for (const QString &error : loaded.errors) {
        displayResult.logMessages.append("Result display failed: " + error);
    }
    if (!loaded.success) {
        return displayResult;
    }

    const QString fieldName = resultObject.displayFieldName.isEmpty()
        ? resultObject.primaryFieldName
        : resultObject.displayFieldName;
    const CalculiXResultGridBuildResult gridResult =
        CalculiXResultGridBuilder().buildResultGrid(
            loaded.meshData,
            loaded.datResult,
            fieldName,
            resultObject.deformationScale
        );
    displayResult.logMessages.append(gridResult.warnings);
    displayResult.logMessages.append(gridResult.errors);
    resultObject.checkMessages.append(gridResult.warnings);
    if (!gridResult.success || !gridResult.grid) {
        return displayResult;
    }

    resultObject.matchedNodeCount = gridResult.matchedNodeCount;
    resultObject.meshNodeCount = gridResult.meshNodeCount;
    resultObject.matchedElementCount = gridResult.matchedElementCount;
    resultObject.meshElementCount = gridResult.meshElementCount;
    resultObject.scalarMin = gridResult.scalarMin;
    resultObject.scalarMax = gridResult.scalarMax;
    resultObject.extrema = ResultExtremaCalculator().calculate(
        loaded.meshData,
        loaded.datResult,
        gridResult.scalarName,
        resultObject.deformationScale
    );
    if (gridResult.matchedNodeCount < gridResult.meshNodeCount) {
        resultObject.checkMessages.append("Node result coverage is incomplete.");
    }
    if (gridResult.scalarName == CalculiXResultFields::VonMisesStress
            && gridResult.matchedElementCount < gridResult.meshElementCount) {
        resultObject.checkMessages.append("Element stress coverage is incomplete.");
    }

    vtkSmartPointer<vtkUnstructuredGrid> overlayGrid;
    if (resultObject.showUndeformedOverlay && resultObject.deformationScale != 0.0) {
        QString overlayError;
        overlayGrid = MeshToVtkConverter::toUnstructuredGrid(loaded.meshData, &overlayError);
        if (!overlayGrid) {
            resultObject.checkMessages.append("Cannot build undeformed overlay: " + overlayError);
        }
    }

    const QString unit = scalarUnit(gridResult.scalarName);
    const QString subtitle = QString("%1 nodes matched, %2 tetrahedra, scale=%3, range [%4, %5] %6")
        .arg(gridResult.matchedNodeCount)
        .arg(loaded.meshData.tetraCount())
        .arg(resultObject.deformationScale, 0, 'g', 6)
        .arg(gridResult.scalarMin, 0, 'g', 6)
        .arg(gridResult.scalarMax, 0, 'g', 6)
        .arg(unit);
    renderView->showResultGrid(
        gridResult.grid,
        overlayGrid,
        resultObject.name,
        subtitle,
        gridResult.scalarName,
        unit,
        gridResult.scalarAssociation == CalculiXResultScalarAssociation::Cell,
        gridResult.scalarMin,
        gridResult.scalarMax,
        resultObject.showMeshEdges,
        resetCamera
    );

    const ResultExtremeMarker &marker = resultObject.extrema.selectedMarker;
    if (marker.valid) {
        renderView->highlightResultPosition(marker.x, marker.y, marker.z);
        const QString idLabel = marker.element ? "maxElement" : "maxNode";
        displayResult.logMessages.append(QString("Result extrema: field=%1, %2=%3, value=%4.")
            .arg(marker.fieldName)
            .arg(idLabel)
            .arg(marker.id)
            .arg(marker.value, 0, 'g', 8));
    }

    displayResult.logMessages.append("Result displayed: " + resultObject.name);
    displayResult.logMessages.append(QString("CalculiX result field: %1, nodes=%2/%3, elements=%4/%5, scale=%6.")
        .arg(gridResult.scalarName)
        .arg(gridResult.matchedNodeCount)
        .arg(gridResult.meshNodeCount)
        .arg(gridResult.matchedElementCount)
        .arg(gridResult.meshElementCount)
        .arg(resultObject.deformationScale, 0, 'g', 6));
    displayResult.success = true;
    return displayResult;
}
