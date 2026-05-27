#include "result/ResultDisplayController.h"

#include "mesh/MeshToVtkConverter.h"
#include "result/ResultDataLoader.h"
#include "result/ResultExtremaCalculator.h"
#include "result/ResultFieldMetadata.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "solver/calculix/CalculiXResultMath.h"
#include "ui/RenderView.h"

#include <QFileInfo>
#include <vtkUnstructuredGrid.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <unordered_map>

namespace
{
bool fileExistsOrEmpty(const QString &filePath)
{
    return !filePath.isEmpty() && QFileInfo::exists(filePath);
}

struct StressAccumulator
{
    double vonMisesSum = 0.0;
    int count = 0;
};

struct ResultDisplaySummary
{
    bool success = false;
    QString scalarName;
    int matchedNodeCount = 0;
    int meshNodeCount = 0;
    int matchedElementCount = 0;
    int meshElementCount = 0;
    double scalarMin = 0.0;
    double scalarMax = 0.0;
    QStringList warnings;
    QStringList errors;
};

bool isDisplacementField(const QString &fieldName)
{
    return fieldName == CalculiXResultFields::Ux
        || fieldName == CalculiXResultFields::Uy
        || fieldName == CalculiXResultFields::Uz
        || fieldName == CalculiXResultFields::DisplacementMagnitude;
}

double displacementScalar(const CalculiXNodeDisplacement &value, const QString &fieldName)
{
    if (fieldName == CalculiXResultFields::Ux) {
        return value.ux;
    }
    if (fieldName == CalculiXResultFields::Uy) {
        return value.uy;
    }
    if (fieldName == CalculiXResultFields::Uz) {
        return value.uz;
    }
    return CalculiXResultMath::displacementMagnitude(value);
}

void updateRange(double value, double &scalarMin, double &scalarMax)
{
    scalarMin = std::min(scalarMin, value);
    scalarMax = std::max(scalarMax, value);
}

ResultDisplaySummary summarizeResultData(
    const MeshData &meshData,
    const CalculiXDatResult &result,
    const QString &fieldName
)
{
    ResultDisplaySummary summary;
    summary.scalarName = fieldName.isEmpty()
        ? QString(CalculiXResultFields::DisplacementMagnitude)
        : fieldName;
    summary.meshNodeCount = meshData.nodeCount();
    summary.meshElementCount = meshData.tetraCount();

    if (meshData.nodes.empty() || meshData.tetraCount() == 0) {
        summary.errors.append("Cannot build result summary: mesh has no nodes or tetrahedra.");
        return summary;
    }
    if (result.displacements.empty() && isDisplacementField(summary.scalarName)) {
        summary.errors.append("Cannot build result summary: displacement field is empty.");
        return summary;
    }
    if (summary.scalarName == CalculiXResultFields::VonMisesStress && result.stresses.empty()) {
        summary.errors.append("Cannot build result summary: stress field is empty.");
        return summary;
    }

    std::unordered_map<int, CalculiXNodeDisplacement> displacementByNodeId;
    displacementByNodeId.reserve(result.displacements.size());
    for (const CalculiXNodeDisplacement &displacement : result.displacements) {
        displacementByNodeId.insert_or_assign(displacement.nodeId, displacement);
    }

    std::unordered_map<int, StressAccumulator> stressByElementId;
    stressByElementId.reserve(result.stresses.size());
    for (const CalculiXElementStress &stress : result.stresses) {
        StressAccumulator &accumulator = stressByElementId[stress.elementId];
        accumulator.vonMisesSum += CalculiXResultMath::vonMisesStress(stress);
        ++accumulator.count;
    }

    double scalarMin = std::numeric_limits<double>::max();
    double scalarMax = std::numeric_limits<double>::lowest();

    for (const MeshNode &node : meshData.nodes) {
        const auto displacementIt = displacementByNodeId.find(node.id);
        if (displacementIt == displacementByNodeId.end()) {
            if (isDisplacementField(summary.scalarName)) {
                updateRange(0.0, scalarMin, scalarMax);
            }
            continue;
        }

        ++summary.matchedNodeCount;
        if (isDisplacementField(summary.scalarName)) {
            updateRange(displacementScalar(displacementIt->second, summary.scalarName), scalarMin, scalarMax);
        }
    }

    const auto updateElementStress = [&](int elementId) {
        double cellStress = 0.0;
        const auto stressIt = stressByElementId.find(elementId);
        if (stressIt != stressByElementId.end() && stressIt->second.count > 0) {
            cellStress = stressIt->second.vonMisesSum / static_cast<double>(stressIt->second.count);
            ++summary.matchedElementCount;
        }
        if (summary.scalarName == CalculiXResultFields::VonMisesStress) {
            updateRange(cellStress, scalarMin, scalarMax);
        }
    };
    for (const TetraElement &element : meshData.tetraElements) {
        updateElementStress(element.id);
    }
    for (const Tetra10Element &element : meshData.tetra10Elements) {
        updateElementStress(element.id);
    }

    if (summary.matchedNodeCount < summary.meshNodeCount) {
        summary.warnings.append(QString("Only %1 of %2 mesh nodes have displacement values; missing nodes use 0.")
            .arg(summary.matchedNodeCount)
            .arg(summary.meshNodeCount));
    }
    if (summary.scalarName == CalculiXResultFields::VonMisesStress
            && summary.matchedElementCount < summary.meshElementCount) {
        summary.warnings.append(QString("Only %1 of %2 tetrahedra have stress values; missing cells use 0.")
            .arg(summary.matchedElementCount)
            .arg(summary.meshElementCount));
    }

    summary.scalarMin = scalarMin == std::numeric_limits<double>::max() ? 0.0 : scalarMin;
    summary.scalarMax = scalarMax == std::numeric_limits<double>::lowest() ? 0.0 : scalarMax;
    summary.success = true;
    return summary;
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
        const ResultDisplaySummary summary = summarizeResultData(loaded->meshData, loaded->datResult, fieldName);
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
