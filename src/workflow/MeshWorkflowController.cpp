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
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

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
    workflowResult.logMessages.append(zh(u8"Gmsh 退出码：%1").arg(gmshResult.exitCode));
    workflowResult.logMessages.append(
        zh(u8"Gmsh 标准输出：") + (gmshResult.standardOutput.isEmpty() ? zh(u8"<空>") : gmshResult.standardOutput)
    );
    workflowResult.logMessages.append(
        zh(u8"Gmsh 错误输出：") + (gmshResult.standardError.isEmpty() ? zh(u8"<空>") : gmshResult.standardError)
    );
    Q_UNUSED(gmshRunner);
}

const GeometryObject *selectedGeometryOrLog(const ProjectModel &projectModel, MeshWorkflowResult &result)
{
    if (!projectModel.hasProject()) {
        result.logMessages.append(zh(u8"网格操作失败：请先创建或打开工程。"));
        return nullptr;
    }

    const GeometryObject *selectedGeometry = projectModel.geometryForSelection();
    if (!selectedGeometry) {
        result.logMessages.append(zh(u8"请先在工程树中选择一个几何对象。"));
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

    result.logMessages.append(zh(u8"MSH 文件：") + meshAbsPath);
    if (!QFileInfo::exists(meshAbsPath)) {
        result.logMessages.append(zh(u8"MSH 文件不存在。"));
        return false;
    }

    meshData.sourceGeometryName = geometry.name;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        result.logMessages.append(zh(u8"无法读取 MSH：") + errorMessage);
        return false;
    }
    return true;
}

QString meshOptionLogSuffix(const MeshSetup &meshSetup)
{
    QString suffix;
    if (meshSetup.elementType == MeshElementType::Tetra10) {
        suffix += " -order 2 -setnumber Mesh.SecondOrderLinear 1";
    }

    if (!meshSetup.autoSize) {
        if (meshSetup.minimumSize > 0.0) {
            suffix += QString(" -clmin %1").arg(meshSetup.minimumSize, 0, 'g', 12);
        }
        if (meshSetup.maximumSize > 0.0) {
            suffix += QString(" -clmax %1").arg(meshSetup.maximumSize, 0, 'g', 12);
        }
    }
    return suffix;
}
}

MeshWorkflowResult MeshWorkflowController::checkGmsh() const
{
    MeshWorkflowResult workflowResult;
    const GmshRunner gmshRunner;
    const GmshRunResult gmshResult = gmshRunner.checkVersion();

    workflowResult.logMessages.append(zh(u8"Gmsh 路径：") + gmshRunner.gmshExecutablePath());
    workflowResult.logMessages.append(zh(u8"Gmsh 命令：") + gmshRunner.gmshExecutablePath() + " --version");
    appendGmshRunLog(workflowResult, gmshRunner, gmshResult);

    if (gmshResult.success) {
        workflowResult.logMessages.append(zh(u8"Gmsh 环境检查通过。"));
    } else {
        workflowResult.logMessages.append(
            gmshResult.errorMessage.isEmpty()
                ? zh(u8"Gmsh 环境检查失败：未知错误。")
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
        workflowResult.logMessages.append(zh(u8"当前几何没有 STEP 文件。"));
        return workflowResult;
    }

    const QString stepAbsPath = absoluteProjectPath(projectModel, geometry.stepFile);
    if (!QFileInfo::exists(stepAbsPath)) {
        workflowResult.logMessages.append(zh(u8"生成网格失败：STEP 文件不存在：") + stepAbsPath);
        return workflowResult;
    }

    const QString safeGeometryName = makeSafeFileBaseName(geometry.name);
    const QString meshRelativePath = QDir("mesh").filePath(safeGeometryName + ".msh");
    const QString meshAbsPath = QDir(projectModel.project().rootPath).filePath(meshRelativePath);

    const GmshCaseWriter gmshCaseWriter;
    const GmshCaseWriterResult gmshCaseResult = gmshCaseWriter.prepareFaceGroupExport(projectModel, geometry);
    workflowResult.logMessages.append(gmshCaseResult.logMessages);
    for (const QString &warning : gmshCaseResult.warnings) {
        workflowResult.logMessages.append(zh(u8"Gmsh 算例警告：") + warning);
    }
    for (const QString &error : gmshCaseResult.errors) {
        workflowResult.logMessages.append(zh(u8"Gmsh 算例失败：") + error);
    }
    if (!gmshCaseResult.errors.isEmpty()) {
        return workflowResult;
    }

    const GmshRunner gmshRunner;
    const QString gmshInputPath = gmshCaseResult.meshInputFile.isEmpty()
        ? stepAbsPath
        : gmshCaseResult.meshInputFile;
    const MeshSetup &meshSetup = projectModel.meshRepository().meshSetup();
    const GmshRunResult gmshResult = gmshRunner.generate3DMesh(gmshInputPath, meshAbsPath, meshSetup);

    workflowResult.logMessages.append(zh(u8"几何名称：") + geometry.name);
    workflowResult.logMessages.append(zh(u8"几何类型：") + geometry.type);
    workflowResult.logMessages.append(zh(u8"网格单元类型：") + displayName(meshSetup.elementType));
    workflowResult.logMessages.append(meshSetup.autoSize
        ? zh(u8"网格尺寸模式：自动")
        : zh(u8"网格尺寸模式：手动，min=%1，max=%2")
            .arg(meshSetup.minimumSize, 0, 'g', 12)
            .arg(meshSetup.maximumSize, 0, 'g', 12));
    workflowResult.logMessages.append(zh(u8"Gmsh 路径：") + gmshRunner.gmshExecutablePath());
    workflowResult.logMessages.append(zh(u8"Gmsh 命令：") + gmshRunner.gmshExecutablePath()
        + " " + gmshInputPath
        + " -3 -format msh2 -o " + meshAbsPath
        + meshOptionLogSuffix(meshSetup));
    workflowResult.logMessages.append(zh(u8"Gmsh 输入：") + gmshInputPath);
    workflowResult.logMessages.append(zh(u8"Gmsh 输出：") + meshAbsPath);
    appendGmshRunLog(workflowResult, gmshRunner, gmshResult);

    if (!gmshResult.success) {
        workflowResult.logMessages.append(
            gmshResult.errorMessage.isEmpty()
                ? zh(u8"生成网格失败：未知错误。")
                : gmshResult.errorMessage
        );
        return workflowResult;
    }

    workflowResult.logMessages.append(zh(u8"网格已生成：") + meshRelativePath);

    MeshData meshData;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        workflowResult.logMessages.append(zh(u8"保存网格对象失败：无法读取生成的 MSH：") + errorMessage);
        return workflowResult;
    }

    MeshObject meshObject;
    meshObject.name = geometry.name + "_Mesh";
    meshObject.sourceGeometryName = geometry.name;
    meshObject.sourceGeometryType = geometry.type;
    meshObject.sourceStepFile = geometry.stepFile;
    meshObject.mshFile = meshRelativePath;
    meshObject.type = toString(meshSetup.elementType);
    meshObject.nodeCount = meshData.nodeCount();
    meshObject.tetraCount = meshData.tetraCount();
    meshObject.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);

    const QVector<MeshBoundary> meshBoundaries = MeshBoundaryBuilder::build(meshData, meshObject);

    MeshManager meshManager(projectModel.project().rootPath);
    if (!meshManager.saveMeshObject(meshObject, &errorMessage)) {
        workflowResult.logMessages.append(zh(u8"保存网格对象失败：") + errorMessage);
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
    workflowResult.logMessages.append(zh(u8"网格对象已保存：mesh/") + safeGeometryName + "_mesh.json");
    workflowResult.logMessages.append(zh(u8"检测到网格边界：%1").arg(meshBoundaries.size()));
    for (const MeshBoundary &meshBoundary : meshBoundaries) {
        workflowResult.logMessages.append(
            zh(u8"网格边界：%1，physicalTag=%2，面数=%3")
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
            workflowResult.logMessages.last().prepend(zh(u8"读取网格失败："));
        }
        return workflowResult;
    }

    workflowResult.logMessages.append(zh(u8"读取网格成功。"));
    workflowResult.logMessages.append(zh(u8"节点数：%1").arg(meshData.nodeCount()));
    workflowResult.logMessages.append(zh(u8"四面体数：%1").arg(meshData.tetraCount()));
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
            workflowResult.logMessages.last().prepend(zh(u8"显示网格失败："));
        }
        return workflowResult;
    }

    QString errorMessage;
    vtkSmartPointer<vtkUnstructuredGrid> grid = MeshToVtkConverter::toUnstructuredGrid(meshData, &errorMessage);
    if (!grid) {
        workflowResult.logMessages.append(zh(u8"VTK 网格转换失败：") + errorMessage);
        return workflowResult;
    }

    if (renderView) {
        const QString subtitle = zh(u8"%1 个节点，%2 个四面体")
            .arg(meshData.nodeCount())
            .arg(meshData.tetraCount());
        renderView->showMeshGrid(grid, selectedGeometry->name + " Mesh", subtitle);
    }
    workflowResult.logMessages.append(zh(u8"网格已显示。"));
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
        workflowResult.logMessages.append(zh(u8"显示网格失败：MSH 文件不存在：") + meshObject.mshFile);
        return workflowResult;
    }

    MeshData meshData;
    meshData.sourceGeometryName = meshObject.sourceGeometryName;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        workflowResult.logMessages.append(zh(u8"显示网格失败：无法读取 MSH：") + errorMessage);
        return workflowResult;
    }

    vtkSmartPointer<vtkUnstructuredGrid> grid = MeshToVtkConverter::toUnstructuredGrid(meshData, &errorMessage);
    if (!grid) {
        workflowResult.logMessages.append(zh(u8"VTK 网格转换失败：") + errorMessage);
        return workflowResult;
    }

    if (renderView) {
        const QString subtitle = zh(u8"%1 个节点，%2 个四面体")
            .arg(meshData.nodeCount())
            .arg(meshData.tetraCount());
        renderView->showMeshGrid(grid, meshObject.name, subtitle);
    }
    workflowResult.logMessages.append(zh(u8"网格已显示：") + meshObject.name);
    return workflowResult;
}
