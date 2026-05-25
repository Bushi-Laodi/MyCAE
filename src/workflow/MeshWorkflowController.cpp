#include "workflow/MeshWorkflowController.h"

#include "geometry/GeometryObject.h"
#include "mesh/GmshCaseWriter.h"
#include "mesh/GmshRunner.h"
#include "mesh/MeshData.h"
#include "mesh/MeshBoundaryBuilder.h"
#include "mesh/MeshManager.h"
#include "mesh/MeshToVtkConverter.h"
#include "mesh/MshReader.h"
#include "project/ProjectModel.h"
#include "ui/RenderView.h"

#include <vtkUnstructuredGrid.h>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

namespace
{
QString makeSafeFileBaseName(const QString &name)
{
    QString result = name.toLower();
    for (QChar &ch : result) {
        if (ch.isSpace()) {
            ch = '_';
        } else if (!ch.isLetterOrNumber() && ch != '_') {
            ch = '_';
        }
    }
    return result.isEmpty() ? QString("geometry") : result;
}

QString absoluteProjectPath(const ProjectModel &projectModel, const QString &path)
{
    return QFileInfo(path).isAbsolute()
        ? path
        : QDir(projectModel.project().rootPath).filePath(path);
}

void appendGmshRunLog(
    MeshWorkflowResult &workflowResult,
    const GmshRunner &gmshRunner,
    const GmshRunResult &gmshResult
)
{
    workflowResult.logMessages.append(QString("Gmsh exitCode: %1").arg(gmshResult.exitCode));
    workflowResult.logMessages.append(
        "Gmsh stdout: " + (gmshResult.standardOutput.isEmpty() ? QString("<empty>") : gmshResult.standardOutput)
    );
    workflowResult.logMessages.append(
        "Gmsh stderr: " + (gmshResult.standardError.isEmpty() ? QString("<empty>") : gmshResult.standardError)
    );
    Q_UNUSED(gmshRunner);
}

const GeometryObject *selectedGeometryOrLog(const ProjectModel &projectModel, MeshWorkflowResult &result)
{
    if (!projectModel.hasProject()) {
        result.logMessages.append("Mesh operation failed: create or open a project first.");
        return nullptr;
    }

    const GeometryObject *selectedGeometry = projectModel.geometryForSelection();
    if (!selectedGeometry) {
        result.logMessages.append("Please select a geometry object in the project tree first.");
        return nullptr;
    }
    return selectedGeometry;
}

bool readMeshDataForGeometry(
    const ProjectModel &projectModel,
    const GeometryObject &geometry,
    MeshData &meshData,
    QString &meshAbsPath,
    MeshWorkflowResult &result
)
{
    const QString meshRelativePath = QDir("mesh").filePath(makeSafeFileBaseName(geometry.name) + ".msh");
    meshAbsPath = QDir(projectModel.project().rootPath).filePath(meshRelativePath);

    result.logMessages.append("MSH file: " + meshAbsPath);
    if (!QFileInfo::exists(meshAbsPath)) {
        result.logMessages.append("MSH file does not exist.");
        return false;
    }

    meshData.sourceGeometryName = geometry.name;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        result.logMessages.append("Cannot read MSH: " + errorMessage);
        return false;
    }
    return true;
}
}

MeshWorkflowResult MeshWorkflowController::checkGmsh() const
{
    MeshWorkflowResult workflowResult;
    const GmshRunner gmshRunner;
    const GmshRunResult gmshResult = gmshRunner.checkVersion();

    workflowResult.logMessages.append("Gmsh path: " + gmshRunner.gmshExecutablePath());
    workflowResult.logMessages.append("Gmsh command: " + gmshRunner.gmshExecutablePath() + " --version");
    appendGmshRunLog(workflowResult, gmshRunner, gmshResult);

    if (gmshResult.success) {
        workflowResult.logMessages.append("Gmsh environment check succeeded.");
    } else {
        workflowResult.logMessages.append(
            gmshResult.errorMessage.isEmpty()
                ? QString("Gmsh environment check failed: unknown error.")
                : gmshResult.errorMessage
        );
    }
    return workflowResult;
}

MeshWorkflowResult MeshWorkflowController::generateMesh(ProjectModel &projectModel) const
{
    MeshWorkflowResult workflowResult;
    const GeometryObject *selectedGeometry = selectedGeometryOrLog(projectModel, workflowResult);
    if (!selectedGeometry) {
        return workflowResult;
    }

    const GeometryObject &geometry = *selectedGeometry;
    if (geometry.stepFile.isEmpty()) {
        workflowResult.logMessages.append("Current geometry has no STEP file.");
        return workflowResult;
    }

    const QString stepAbsPath = absoluteProjectPath(projectModel, geometry.stepFile);
    if (!QFileInfo::exists(stepAbsPath)) {
        workflowResult.logMessages.append("Generate mesh failed: STEP file does not exist: " + stepAbsPath);
        return workflowResult;
    }

    const QString safeGeometryName = makeSafeFileBaseName(geometry.name);
    const QString meshRelativePath = QDir("mesh").filePath(safeGeometryName + ".msh");
    const QString meshAbsPath = QDir(projectModel.project().rootPath).filePath(meshRelativePath);

    const GmshCaseWriter gmshCaseWriter;
    const GmshCaseWriterResult gmshCaseResult = gmshCaseWriter.prepareFaceGroupExport(projectModel, geometry);
    workflowResult.logMessages.append(gmshCaseResult.logMessages);
    for (const QString &warning : gmshCaseResult.warnings) {
        workflowResult.logMessages.append("Gmsh case warning: " + warning);
    }
    for (const QString &error : gmshCaseResult.errors) {
        workflowResult.logMessages.append("Gmsh case failed: " + error);
    }
    if (!gmshCaseResult.errors.isEmpty()) {
        return workflowResult;
    }

    const GmshRunner gmshRunner;
    const QString gmshInputPath = gmshCaseResult.meshInputFile.isEmpty()
        ? stepAbsPath
        : gmshCaseResult.meshInputFile;
    const GmshRunResult gmshResult = gmshRunner.generate3DMesh(gmshInputPath, meshAbsPath);

    workflowResult.logMessages.append("Geometry name: " + geometry.name);
    workflowResult.logMessages.append("Geometry type: " + geometry.type);
    workflowResult.logMessages.append("Gmsh path: " + gmshRunner.gmshExecutablePath());
    workflowResult.logMessages.append("Gmsh command: " + gmshRunner.gmshExecutablePath()
        + " " + gmshInputPath
        + " -3 -format msh2 -o " + meshAbsPath);
    workflowResult.logMessages.append("Gmsh input: " + gmshInputPath);
    workflowResult.logMessages.append("Gmsh output: " + meshAbsPath);
    appendGmshRunLog(workflowResult, gmshRunner, gmshResult);

    if (!gmshResult.success) {
        workflowResult.logMessages.append(
            gmshResult.errorMessage.isEmpty()
                ? QString("Generate mesh failed: unknown error.")
                : gmshResult.errorMessage
        );
        return workflowResult;
    }

    workflowResult.logMessages.append("Mesh generated: " + meshRelativePath);

    MeshData meshData;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        workflowResult.logMessages.append("MeshObject save failed: cannot read generated MSH: " + errorMessage);
        return workflowResult;
    }

    MeshObject meshObject;
    meshObject.name = geometry.name + "_Mesh";
    meshObject.sourceGeometryName = geometry.name;
    meshObject.sourceGeometryType = geometry.type;
    meshObject.sourceStepFile = geometry.stepFile;
    meshObject.mshFile = meshRelativePath;
    meshObject.type = "tetra4";
    meshObject.nodeCount = meshData.nodeCount();
    meshObject.tetraCount = meshData.tetraCount();
    meshObject.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);

    const QVector<MeshBoundary> meshBoundaries = MeshBoundaryBuilder::build(meshData, meshObject);

    MeshManager meshManager(projectModel.project().rootPath);
    if (!meshManager.saveMeshObject(meshObject, &errorMessage)) {
        workflowResult.logMessages.append("MeshObject save failed: " + errorMessage);
        return workflowResult;
    }

    bool replaced = false;
    QVector<MeshObject> &meshObjects = projectModel.meshRepository().meshObjects();
    for (MeshObject &existingMesh : meshObjects) {
        if (existingMesh.sourceGeometryName == meshObject.sourceGeometryName) {
            existingMesh = meshObject;
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        meshObjects.append(meshObject);
    }
    projectModel.meshRepository().replaceMeshBoundariesForMesh(meshObject.name, meshBoundaries);
    projectModel.setSelection(Selection::item(SelectionKind::Mesh, meshObject.name, meshObject.name));

    workflowResult.meshTreeChanged = true;
    workflowResult.simulationCaseChanged = true;
    workflowResult.logMessages.append("MeshObject saved: mesh/" + safeGeometryName + "_mesh.json");
    workflowResult.logMessages.append(QString("Mesh boundaries detected: %1").arg(meshBoundaries.size()));
    for (const MeshBoundary &meshBoundary : meshBoundaries) {
        workflowResult.logMessages.append(
            QString("MeshBoundary: %1, physicalTag=%2, faces=%3")
                .arg(meshBoundary.sourceFaceGroupId)
                .arg(meshBoundary.physicalGroupTag)
                .arg(meshBoundary.faceCount)
        );
    }
    return workflowResult;
}

MeshWorkflowResult MeshWorkflowController::readMeshInfo(const ProjectModel &projectModel) const
{
    MeshWorkflowResult workflowResult;
    const GeometryObject *selectedGeometry = selectedGeometryOrLog(projectModel, workflowResult);
    if (!selectedGeometry) {
        return workflowResult;
    }

    MeshData meshData;
    QString meshAbsPath;
    if (!readMeshDataForGeometry(projectModel, *selectedGeometry, meshData, meshAbsPath, workflowResult)) {
        if (!workflowResult.logMessages.isEmpty()) {
            workflowResult.logMessages.last().prepend("Read mesh failed: ");
        }
        return workflowResult;
    }

    workflowResult.logMessages.append("Read mesh succeeded.");
    workflowResult.logMessages.append(QString("Node count: %1").arg(meshData.nodeCount()));
    workflowResult.logMessages.append(QString("Tetra count: %1").arg(meshData.tetraCount()));
    return workflowResult;
}

MeshWorkflowResult MeshWorkflowController::showSelectedGeometryMesh(
    const ProjectModel &projectModel,
    RenderView *renderView
) const
{
    MeshWorkflowResult workflowResult;
    const GeometryObject *selectedGeometry = selectedGeometryOrLog(projectModel, workflowResult);
    if (!selectedGeometry) {
        return workflowResult;
    }

    MeshData meshData;
    QString meshAbsPath;
    if (!readMeshDataForGeometry(projectModel, *selectedGeometry, meshData, meshAbsPath, workflowResult)) {
        if (!workflowResult.logMessages.isEmpty()) {
            workflowResult.logMessages.last().prepend("Show mesh failed: ");
        }
        return workflowResult;
    }

    QString errorMessage;
    vtkSmartPointer<vtkUnstructuredGrid> grid = MeshToVtkConverter::toUnstructuredGrid(meshData, &errorMessage);
    if (!grid) {
        workflowResult.logMessages.append("VTK mesh conversion failed: " + errorMessage);
        return workflowResult;
    }

    if (renderView) {
        const QString subtitle = QString("%1 nodes, %2 tetrahedra")
            .arg(meshData.nodeCount())
            .arg(meshData.tetraCount());
        renderView->showMeshGrid(grid, selectedGeometry->name + " Mesh", subtitle);
    }
    workflowResult.logMessages.append("Mesh displayed.");
    return workflowResult;
}

MeshWorkflowResult MeshWorkflowController::displayMeshObject(
    const ProjectModel &projectModel,
    const MeshObject &meshObject,
    RenderView *renderView
) const
{
    MeshWorkflowResult workflowResult;
    const QString meshAbsPath = absoluteProjectPath(projectModel, meshObject.mshFile);
    if (!QFileInfo::exists(meshAbsPath)) {
        workflowResult.logMessages.append("Show mesh failed: MSH file does not exist: " + meshObject.mshFile);
        return workflowResult;
    }

    MeshData meshData;
    meshData.sourceGeometryName = meshObject.sourceGeometryName;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        workflowResult.logMessages.append("Show mesh failed: cannot read MSH: " + errorMessage);
        return workflowResult;
    }

    vtkSmartPointer<vtkUnstructuredGrid> grid = MeshToVtkConverter::toUnstructuredGrid(meshData, &errorMessage);
    if (!grid) {
        workflowResult.logMessages.append("VTK mesh conversion failed: " + errorMessage);
        return workflowResult;
    }

    if (renderView) {
        const QString subtitle = QString("%1 nodes, %2 tetrahedra")
            .arg(meshData.nodeCount())
            .arg(meshData.tetraCount());
        renderView->showMeshGrid(grid, meshObject.name, subtitle);
    }
    workflowResult.logMessages.append("Mesh displayed: " + meshObject.name);
    return workflowResult;
}
