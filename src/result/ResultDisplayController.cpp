#include "result/ResultDisplayController.h"

#include "mesh/MeshToVtkConverter.h"
#include "result/ResultDataLoader.h"
#include "result/ResultExtremaCalculator.h"
#include "result/ResultFieldMetadata.h"
#include "result/ResultDisplaySummary.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "ui/RenderView.h"

#include <QFileInfo>
#include <vtkUnstructuredGrid.h>

#include <algorithm>
#include <memory>

namespace
{
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
    auto loaded = std::make_unique<ResultDataLoadResult>(ResultDataLoader().loadCalculiXResult(projectModel, resultObject));
    resultObject.checkMessages.append(loaded->warnings);
    displayResult.logMessages.append(loaded->warnings);
    for (const QString &error : loaded->errors) {
        displayResult.logMessages.append("Result display failed: " + error);
    }
    if (!loaded->success) {
        return displayResult;
    }

    const QString fieldName = resultObject.displayFieldName.isEmpty()
        ? resultObject.primaryFieldName
        : resultObject.displayFieldName;
    const bool summaryOnly = renderView->property("mycae.skipResultRender").toBool();
    if (summaryOnly) {
        const ResultDisplaySummary summary =
            ResultDisplaySummarizer().summarize(loaded->meshData, loaded->datResult, fieldName);
        displayResult.logMessages.append(summary.warnings);
        displayResult.logMessages.append(summary.errors);
        resultObject.checkMessages.append(summary.warnings);
        if (!summary.success) {
            return displayResult;
        }

        resultObject.matchedNodeCount = summary.matchedNodeCount;
        resultObject.meshNodeCount = summary.meshNodeCount;
        resultObject.matchedElementCount = summary.matchedElementCount;
        resultObject.meshElementCount = summary.meshElementCount;
        resultObject.scalarMin = summary.scalarMin;
        resultObject.scalarMax = summary.scalarMax;
        if (!resultObject.scalarRangeLocked) {
            resultObject.lockedScalarMin = summary.scalarMin;
            resultObject.lockedScalarMax = summary.scalarMax;
        }
        resultObject.extrema = {};
        if (summary.matchedNodeCount < summary.meshNodeCount) {
            resultObject.checkMessages.append("Node result coverage is incomplete.");
        }
        if (summary.scalarName == CalculiXResultFields::VonMisesStress
                && summary.matchedElementCount < summary.meshElementCount) {
            resultObject.checkMessages.append("Element stress coverage is incomplete.");
        }

        const ResultExtremeMarker &minimumMarker = resultObject.extrema.selectedMinimumMarker;
        const ResultExtremeMarker &maximumMarker = resultObject.extrema.selectedMaximumMarker;
        if (minimumMarker.valid) {
            const QString idLabel = minimumMarker.element ? "minElement" : "minNode";
            displayResult.logMessages.append(QString("Result minimum: field=%1, %2=%3, value=%4.")
                .arg(minimumMarker.fieldName)
                .arg(idLabel)
                .arg(minimumMarker.id)
                .arg(minimumMarker.value, 0, 'g', 8));
        }
        if (maximumMarker.valid) {
            const QString idLabel = maximumMarker.element ? "maxElement" : "maxNode";
            displayResult.logMessages.append(QString("Result maximum: field=%1, %2=%3, value=%4.")
                .arg(maximumMarker.fieldName)
                .arg(idLabel)
                .arg(maximumMarker.id)
                .arg(maximumMarker.value, 0, 'g', 8));
        }
        displayResult.logMessages.append("Result displayed: " + resultObject.name);
        displayResult.logMessages.append(QString("CalculiX result field: %1, nodes=%2/%3, elements=%4/%5, scale=%6.")
            .arg(summary.scalarName)
            .arg(summary.matchedNodeCount)
            .arg(summary.meshNodeCount)
            .arg(summary.matchedElementCount)
            .arg(summary.meshElementCount)
            .arg(resultObject.deformationScale, 0, 'g', 6));
        displayResult.success = true;
        // UI validation runs without a real VTK canvas. In that short-lived mode, keep the
        // loaded mesh/result buffers alive until process exit to avoid teardown-time crashes
        // from debug VTK/Qt dependencies while still validating the postprocess controls.
        loaded.release();
        return displayResult;
    }

    CalculiXResultGridBuildResult gridResult = CalculiXResultGridBuilder().buildResultGrid(
        loaded->meshData,
        loaded->datResult,
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
    const QString unit = ResultFieldMetadata::unitForField(gridResult.scalarName);
    if (!resultObject.scalarRangeLocked) {
        resultObject.lockedScalarMin = gridResult.scalarMin;
        resultObject.lockedScalarMax = gridResult.scalarMax;
    }
    double displayScalarMin = resultObject.scalarRangeLocked ? resultObject.lockedScalarMin : gridResult.scalarMin;
    double displayScalarMax = resultObject.scalarRangeLocked ? resultObject.lockedScalarMax : gridResult.scalarMax;
    if (displayScalarMin > displayScalarMax) {
        std::swap(displayScalarMin, displayScalarMax);
    }
    resultObject.extrema = ResultExtremaCalculator().calculate(
        loaded->meshData,
        loaded->datResult,
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
        overlayGrid = MeshToVtkConverter::toUnstructuredGrid(loaded->meshData, &overlayError);
        if (!overlayGrid) {
            resultObject.checkMessages.append("Cannot build undeformed overlay: " + overlayError);
        }
    }

    if (!summaryOnly && renderView->isVisible()) {
        const QString rangeMode = resultObject.scalarRangeLocked ? "locked" : "auto";
        const QString subtitle = QString("%1 nodes matched, %2 tetrahedra, scale=%3, %4 range [%5, %6] %7")
            .arg(gridResult.matchedNodeCount)
            .arg(loaded->meshData.tetraCount())
            .arg(resultObject.deformationScale, 0, 'g', 6)
            .arg(rangeMode)
            .arg(displayScalarMin, 0, 'g', 6)
            .arg(displayScalarMax, 0, 'g', 6)
            .arg(unit);
        renderView->showResultGrid(
            gridResult.grid,
            overlayGrid,
            resultObject.name,
            subtitle,
            gridResult.scalarName,
            unit,
            gridResult.scalarAssociation == CalculiXResultScalarAssociation::Cell,
            displayScalarMin,
            displayScalarMax,
            resultObject.showMeshEdges,
            resultObject.extrema.selectedMinimumMarker,
            resultObject.extrema.selectedMaximumMarker,
            resetCamera
        );
    }

    const ResultExtremeMarker &minimumMarker = resultObject.extrema.selectedMinimumMarker;
    const ResultExtremeMarker &maximumMarker = resultObject.extrema.selectedMaximumMarker;
    if (minimumMarker.valid || maximumMarker.valid) {
        if (minimumMarker.valid) {
            const QString idLabel = minimumMarker.element ? "minElement" : "minNode";
            displayResult.logMessages.append(QString("Result minimum: field=%1, %2=%3, value=%4.")
                .arg(minimumMarker.fieldName)
                .arg(idLabel)
                .arg(minimumMarker.id)
                .arg(minimumMarker.value, 0, 'g', 8));
        }
        if (maximumMarker.valid) {
            const QString idLabel = maximumMarker.element ? "maxElement" : "maxNode";
            displayResult.logMessages.append(QString("Result maximum: field=%1, %2=%3, value=%4.")
                .arg(maximumMarker.fieldName)
                .arg(idLabel)
                .arg(maximumMarker.id)
                .arg(maximumMarker.value, 0, 'g', 8));
        }
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
