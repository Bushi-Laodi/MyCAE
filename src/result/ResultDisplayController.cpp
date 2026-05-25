#include "result/ResultDisplayController.h"

#include "mesh/MeshData.h"
#include "mesh/MeshObject.h"
#include "mesh/MeshToVtkConverter.h"
#include "mesh/MshReader.h"
#include "project/ProjectModel.h"
#include "solver/calculix/CalculiXDatResultReader.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "ui/RenderView.h"

#include <QDir>
#include <QFileInfo>
#include <vtkUnstructuredGrid.h>

namespace
{
QString absoluteProjectPath(const ProjectModel &projectModel, const QString &path)
{
    return QFileInfo(path).isAbsolute()
        ? path
        : QDir(projectModel.project().rootPath).filePath(path);
}

const MeshObject *resultMeshOrFallback(const ProjectModel &projectModel, const ResultObject &resultObject)
{
    if (!resultObject.meshName.isEmpty()) {
        if (const MeshObject *meshObject = projectModel.findMeshByName(resultObject.meshName)) {
            return meshObject;
        }
    }

    const QVector<MeshObject> &meshes = projectModel.meshObjects();
    return meshes.size() == 1 ? &meshes.front() : nullptr;
}

QString firstExistingDatFile(const ResultObject &resultObject)
{
    if (!resultObject.datFile.isEmpty() && QFileInfo::exists(resultObject.datFile)) {
        return resultObject.datFile;
    }

    const QDir caseDir(resultObject.casePath);
    const QFileInfoList datFiles = caseDir.entryInfoList(
        QStringList{"*.dat"},
        QDir::Files,
        QDir::Time
    );
    return datFiles.isEmpty() ? QString() : datFiles.front().absoluteFilePath();
}

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
    RenderView *renderView
) const
{
    ResultDisplayResult displayResult;
    resultObject.checkMessages = fileCompletenessMessages(resultObject);
    resultObject.resultFilesComplete = resultObject.checkMessages.isEmpty();
    if (!renderView) {
        displayResult.logMessages.append("Result display skipped: render view is not available.");
        return displayResult;
    }
    if (!resultObject.solverName.contains("CalculiX", Qt::CaseInsensitive)) {
        displayResult.logMessages.append("Result display skipped: only CalculiX results are supported currently.");
        return displayResult;
    }

    const MeshObject *meshObject = resultMeshOrFallback(projectModel, resultObject);
    if (!meshObject) {
        displayResult.logMessages.append(
            "Result display failed: result mesh is not loaded or cannot be inferred: " + resultObject.meshName
        );
        return displayResult;
    }

    const QString meshAbsPath = absoluteProjectPath(projectModel, meshObject->mshFile);
    if (!QFileInfo::exists(meshAbsPath)) {
        displayResult.logMessages.append("Result display failed: MSH file does not exist: " + meshAbsPath);
        return displayResult;
    }

    MeshData meshData;
    meshData.sourceGeometryName = meshObject->sourceGeometryName;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        displayResult.logMessages.append("Result display failed: cannot read mesh: " + errorMessage);
        return displayResult;
    }

    const QString datFile = firstExistingDatFile(resultObject);
    if (datFile.isEmpty()) {
        displayResult.logMessages.append("Result display failed: no CalculiX .dat file found in " + resultObject.casePath);
        return displayResult;
    }

    const CalculiXDatReadResult datReadResult = CalculiXDatResultReader().read(datFile);
    resultObject.checkMessages.append(datReadResult.warnings);
    displayResult.logMessages.append(datReadResult.warnings);
    displayResult.logMessages.append(datReadResult.errors);
    if (!datReadResult.success) {
        return displayResult;
    }

    const QString fieldName = resultObject.displayFieldName.isEmpty()
        ? resultObject.primaryFieldName
        : resultObject.displayFieldName;
    const CalculiXResultGridBuildResult gridResult =
        CalculiXResultGridBuilder().buildResultGrid(
            meshData,
            datReadResult.result,
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
        overlayGrid = MeshToVtkConverter::toUnstructuredGrid(meshData, &overlayError);
        if (!overlayGrid) {
            resultObject.checkMessages.append("Cannot build undeformed overlay: " + overlayError);
        }
    }

    const QString unit = scalarUnit(gridResult.scalarName);
    const QString subtitle = QString("%1 nodes matched, %2 tetrahedra, scale=%3, range [%4, %5] %6")
        .arg(gridResult.matchedNodeCount)
        .arg(meshData.tetraCount())
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
        resultObject.showMeshEdges
    );

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
