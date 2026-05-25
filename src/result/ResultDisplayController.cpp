#include "result/ResultDisplayController.h"

#include "mesh/MeshData.h"
#include "mesh/MeshObject.h"
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
}

ResultDisplayResult ResultDisplayController::displayResult(
    const ProjectModel &projectModel,
    const ResultObject &resultObject,
    RenderView *renderView
) const
{
    ResultDisplayResult displayResult;
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
    displayResult.logMessages.append(datReadResult.warnings);
    displayResult.logMessages.append(datReadResult.errors);
    if (!datReadResult.success) {
        return displayResult;
    }

    const CalculiXResultGridBuildResult gridResult =
        CalculiXResultGridBuilder().buildDisplacementGrid(meshData, datReadResult.result);
    displayResult.logMessages.append(gridResult.warnings);
    displayResult.logMessages.append(gridResult.errors);
    if (!gridResult.success || !gridResult.grid) {
        return displayResult;
    }

    const QString subtitle = QString("%1 nodes matched, %2 tetrahedra, |U| range [%3, %4]")
        .arg(gridResult.matchedNodeCount)
        .arg(meshData.tetraCount())
        .arg(gridResult.scalarMin, 0, 'g', 6)
        .arg(gridResult.scalarMax, 0, 'g', 6);
    renderView->showResultGrid(
        gridResult.grid,
        resultObject.name,
        subtitle,
        gridResult.scalarName,
        gridResult.scalarMin,
        gridResult.scalarMax
    );

    displayResult.logMessages.append("Result displayed: " + resultObject.name);
    displayResult.logMessages.append(QString("CalculiX displacement field: %1/%2 nodes, scalar=%3.")
        .arg(gridResult.matchedNodeCount)
        .arg(gridResult.meshNodeCount)
        .arg(gridResult.scalarName));
    displayResult.success = true;
    return displayResult;
}
