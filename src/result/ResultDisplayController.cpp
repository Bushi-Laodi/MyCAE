#include "result/ResultDisplayController.h"

#include "result/ResultDisplayCache.h"
#include "result/ResultDataLoader.h"
#include "result/ResultFieldMetadata.h"
#include "result/ResultDisplaySummary.h"
#include "ui/RenderView.h"

#include <QFileInfo>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>

#include <algorithm>

namespace
{
bool fileExistsOrEmpty(const QString &filePath)
{
    return !filePath.isEmpty() && QFileInfo::exists(filePath);
}

bool isVtkResult(const ResultObject &resultObject)
{
    if (resultObject.solverName.compare("OpenFOAM", Qt::CaseInsensitive) == 0) {
        return true;
    }
    for (const QString &filePath : resultObject.resultFiles) {
        if (QFileInfo(filePath).suffix().compare("vtk", Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

QString firstExistingVtkFile(const ResultObject &resultObject)
{
    for (const QString &filePath : resultObject.resultFiles) {
        const QFileInfo info(filePath);
        if (info.suffix().compare("vtk", Qt::CaseInsensitive) == 0 && info.exists()) {
            return filePath;
        }
    }
    return QString();
}

QStringList fileCompletenessMessages(const ResultObject &resultObject)
{
    QStringList messages;
    if (isVtkResult(resultObject)) {
        if (firstExistingVtkFile(resultObject).isEmpty()) {
            messages.append("Missing .vtk result file.");
        }
        if (!fileExistsOrEmpty(resultObject.logFile)) {
            messages.append("Missing OpenFOAM service log file.");
        }
        return messages;
    }

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

QStringList arrayNames(vtkDataSetAttributes *attributes)
{
    QStringList names;
    if (!attributes) {
        return names;
    }
    for (int i = 0; i < attributes->GetNumberOfArrays(); ++i) {
        vtkDataArray *array = attributes->GetArray(i);
        if (array && array->GetName()) {
            names.append(QString::fromUtf8(array->GetName()));
        }
    }
    return names;
}

vtkDataArray *firstScalarArray(vtkDataSetAttributes *attributes, QString &arrayName)
{
    if (!attributes) {
        return nullptr;
    }
    vtkDataArray *preferred = attributes->GetArray("pressure");
    if (preferred) {
        arrayName = "pressure";
        return preferred;
    }
    preferred = attributes->GetArray("p");
    if (preferred) {
        arrayName = "p";
        return preferred;
    }
    for (int i = 0; i < attributes->GetNumberOfArrays(); ++i) {
        vtkDataArray *array = attributes->GetArray(i);
        if (array && array->GetNumberOfComponents() == 1 && array->GetName()) {
            arrayName = QString::fromUtf8(array->GetName());
            return array;
        }
    }
    return nullptr;
}

vtkDataArray *vtkResultArray(
    vtkUnstructuredGrid *grid,
    const QString &requestedFieldName,
    QString &selectedFieldName,
    bool &useCellScalars
)
{
    if (!grid) {
        return nullptr;
    }
    if (!requestedFieldName.isEmpty() && grid->GetPointData()) {
        vtkDataArray *array = grid->GetPointData()->GetArray(requestedFieldName.toUtf8().constData());
        if (array) {
            selectedFieldName = requestedFieldName;
            useCellScalars = false;
            return array;
        }
    }
    if (!requestedFieldName.isEmpty() && grid->GetCellData()) {
        vtkDataArray *array = grid->GetCellData()->GetArray(requestedFieldName.toUtf8().constData());
        if (array) {
            selectedFieldName = requestedFieldName;
            useCellScalars = true;
            return array;
        }
    }

    if (vtkDataArray *array = firstScalarArray(grid->GetPointData(), selectedFieldName)) {
        useCellScalars = false;
        return array;
    }
    if (vtkDataArray *array = firstScalarArray(grid->GetCellData(), selectedFieldName)) {
        useCellScalars = true;
        return array;
    }
    return nullptr;
}

bool isDisplacementField(const QString &fieldName)
{
    return fieldName == CalculiXResultFields::Ux
        || fieldName == CalculiXResultFields::Uy
        || fieldName == CalculiXResultFields::Uz
        || fieldName == CalculiXResultFields::DisplacementMagnitude;
}

CalculiXResultScalarAssociation associationForField(const QString &fieldName)
{
    return fieldName == CalculiXResultFields::VonMisesStress
        ? CalculiXResultScalarAssociation::Cell
        : CalculiXResultScalarAssociation::Point;
}

vtkDataArray *scalarArray(vtkUnstructuredGrid *grid, const QString &fieldName)
{
    if (!grid) {
        return nullptr;
    }
    if (associationForField(fieldName) == CalculiXResultScalarAssociation::Cell) {
        return grid->GetCellData() ? grid->GetCellData()->GetArray(fieldName.toUtf8().constData()) : nullptr;
    }
    return grid->GetPointData() ? grid->GetPointData()->GetArray(fieldName.toUtf8().constData()) : nullptr;
}

bool scalarRange(vtkUnstructuredGrid *grid, const QString &fieldName, double &minimum, double &maximum)
{
    vtkDataArray *array = scalarArray(grid, fieldName);
    if (!array || array->GetNumberOfTuples() <= 0) {
        return false;
    }

    double range[2] = {0.0, 0.0};
    array->GetRange(range);
    minimum = range[0];
    maximum = range[1];
    return true;
}

QString resultCacheKey(const ResultObject &resultObject)
{
    return resultObject.id + "|" + resultObject.name;
}

ResultDisplayResult displayVtkResult(ResultObject &resultObject, RenderView *renderView, bool resetCamera)
{
    ResultDisplayResult displayResult;
    const QString vtkFile = firstExistingVtkFile(resultObject);
    if (vtkFile.isEmpty()) {
        displayResult.logMessages.append("OpenFOAM result display failed: .vtk file is missing.");
        return displayResult;
    }

    vtkNew<vtkUnstructuredGridReader> reader;
    reader->SetFileName(QFileInfo(vtkFile).absoluteFilePath().toLocal8Bit().constData());
    reader->Update();

    vtkUnstructuredGrid *readerOutput = reader->GetOutput();
    if (!readerOutput || readerOutput->GetNumberOfPoints() <= 0 || readerOutput->GetNumberOfCells() <= 0) {
        displayResult.logMessages.append("OpenFOAM result display failed: VTK file did not contain an unstructured grid.");
        return displayResult;
    }

    vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid->DeepCopy(readerOutput);

    QStringList fields = arrayNames(grid->GetPointData());
    fields.append(arrayNames(grid->GetCellData()));
    fields.removeDuplicates();
    resultObject.availableFields = fields;

    const QString requestedFieldName = resultObject.displayFieldName.isEmpty()
        ? resultObject.primaryFieldName
        : resultObject.displayFieldName;
    QString selectedFieldName;
    bool useCellScalars = false;
    vtkDataArray *array = vtkResultArray(grid, requestedFieldName, selectedFieldName, useCellScalars);

    resultObject.meshNodeCount = static_cast<int>(grid->GetNumberOfPoints());
    resultObject.matchedNodeCount = resultObject.meshNodeCount;
    resultObject.meshElementCount = static_cast<int>(grid->GetNumberOfCells());
    resultObject.matchedElementCount = resultObject.meshElementCount;

    if (!array) {
        renderView->showMeshGrid(
            grid,
            resultObject.name,
            QString("OpenFOAM VTK result, points=%1, cells=%2.")
                .arg(resultObject.meshNodeCount)
                .arg(resultObject.meshElementCount)
        );
        displayResult.logMessages.append("OpenFOAM VTK result displayed without scalar field: " + vtkFile);
        displayResult.success = true;
        return displayResult;
    }

    double range[2] = {0.0, 0.0};
    array->GetRange(range);
    resultObject.primaryFieldName = resultObject.primaryFieldName.isEmpty() ? selectedFieldName : resultObject.primaryFieldName;
    resultObject.displayFieldName = selectedFieldName;
    resultObject.scalarMin = range[0];
    resultObject.scalarMax = range[1];
    if (!resultObject.scalarRangeLocked) {
        resultObject.lockedScalarMin = range[0];
        resultObject.lockedScalarMax = range[1];
    }

    double displayScalarMin = resultObject.scalarRangeLocked ? resultObject.lockedScalarMin : resultObject.scalarMin;
    double displayScalarMax = resultObject.scalarRangeLocked ? resultObject.lockedScalarMax : resultObject.scalarMax;
    if (displayScalarMin > displayScalarMax) {
        std::swap(displayScalarMin, displayScalarMax);
    }

    const bool pressureField = selectedFieldName.compare("pressure", Qt::CaseInsensitive) == 0
        || selectedFieldName.compare("p", Qt::CaseInsensitive) == 0;
    const QString unit = pressureField ? QString("Pa") : QString();
    vtkSmartPointer<vtkUnstructuredGrid> overlayGrid;
    renderView->showResultGrid(
        grid,
        overlayGrid,
        resultObject.name,
        QString("OpenFOAM VTK field=%1, points=%2, cells=%3.")
            .arg(selectedFieldName)
            .arg(resultObject.meshNodeCount)
            .arg(resultObject.meshElementCount),
        selectedFieldName,
        unit,
        useCellScalars,
        displayScalarMin,
        displayScalarMax,
        resultObject.showMeshEdges,
        ResultExtremeMarker{},
        ResultExtremeMarker{},
        resetCamera
    );

    displayResult.logMessages.append("OpenFOAM VTK result displayed: " + vtkFile);
    displayResult.logMessages.append(QString("OpenFOAM result field: %1, points=%2, cells=%3.")
        .arg(selectedFieldName)
        .arg(resultObject.meshNodeCount)
        .arg(resultObject.meshElementCount));
    displayResult.success = true;
    return displayResult;
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
    if (isVtkResult(resultObject)) {
        return displayVtkResult(resultObject, renderView, resetCamera);
    }

    ResultDisplayCacheData cachedData = ResultDisplayCache::instance().loadData(projectModel, resultObject);
    const std::shared_ptr<ResultDataLoadResult> loaded = cachedData.loaded;
    if (!loaded) {
        displayResult.logMessages.append("Result display failed: cache did not return result data.");
        return displayResult;
    }
    resultObject.checkMessages.append(loaded->warnings);
    if (!cachedData.cacheHit) {
        displayResult.logMessages.append(loaded->warnings);
    }
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
        return displayResult;
    }

    if (isDisplacementField(fieldName) && loaded->datResult.displacements.empty()) {
        displayResult.logMessages.append("Result display failed: displacement field is empty.");
        return displayResult;
    }
    if (fieldName == CalculiXResultFields::VonMisesStress && loaded->datResult.stresses.empty()) {
        displayResult.logMessages.append("Result display failed: stress field is empty.");
        return displayResult;
    }

    ResultDisplayCacheGrid cachedGrid = ResultDisplayCache::instance().buildGrid(
        *loaded,
        resultCacheKey(resultObject),
        resultObject.deformationScale
    );
    CalculiXResultGridBuildResult gridResult = cachedGrid.gridResult;
    if (!cachedGrid.cacheHit) {
        displayResult.logMessages.append(gridResult.warnings);
    }
    displayResult.logMessages.append(gridResult.errors);
    resultObject.checkMessages.append(gridResult.warnings);
    if (!gridResult.success || !gridResult.grid) {
        return displayResult;
    }

    gridResult.scalarName = fieldName;
    gridResult.scalarAssociation = associationForField(fieldName);
    if (!scalarRange(gridResult.grid, fieldName, gridResult.scalarMin, gridResult.scalarMax)) {
        displayResult.logMessages.append("Result display failed: scalar array not found: " + fieldName);
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
    resultObject.extrema = ResultDisplayCache::instance().calculateExtrema(
        *loaded,
        resultCacheKey(resultObject),
        gridResult.scalarName,
        resultObject.deformationScale
    ).extrema;
    if (gridResult.matchedNodeCount < gridResult.meshNodeCount) {
        resultObject.checkMessages.append("Node result coverage is incomplete.");
    }
    if (gridResult.scalarName == CalculiXResultFields::VonMisesStress
            && gridResult.matchedElementCount < gridResult.meshElementCount) {
        resultObject.checkMessages.append("Element stress coverage is incomplete.");
    }

    vtkSmartPointer<vtkUnstructuredGrid> overlayGrid;
    if (resultObject.showUndeformedOverlay && resultObject.deformationScale != 0.0) {
        const ResultDisplayCacheOverlay cachedOverlay =
            ResultDisplayCache::instance().buildOverlay(*loaded, resultCacheKey(resultObject));
        overlayGrid = cachedOverlay.grid;
        resultObject.checkMessages.append(cachedOverlay.warnings);
        if (!cachedOverlay.cacheHit) {
            displayResult.logMessages.append(cachedOverlay.warnings);
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
