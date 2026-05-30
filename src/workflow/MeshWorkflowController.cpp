#include "workflow/MeshWorkflowController.h"

#include "geometry/FaceGroup.h"
#include "geometry/FaceGroupService.h"
#include "geometry/GeometryObject.h"
#include "mesh/GmshCaseWriter.h"
#include "mesh/GmshRunner.h"
#include "mesh/MeshData.h"
#include "mesh/MeshBoundaryBuilder.h"
#include "mesh/MeshManager.h"
#include "mesh/MeshQualityChecker.h"
#include "mesh/MeshQualityService.h"
#include "mesh/MeshToVtkConverter.h"
#include "mesh/MshReader.h"
#include "project/ProjectModel.h"
#include "units/UnitConverter.h"
#include "ui/RenderView.h"

#include <vtkUnstructuredGrid.h>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

#include <algorithm>

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

QString nextMeshNameForGeometry(const ProjectModel &projectModel, const QString &geometryName)
{
    const QString baseName = geometryName + "_Mesh";
    bool baseNameExists = false;
    int maxSuffix = 0;

    for (const MeshObject &meshObject : projectModel.meshRepository().meshObjects()) {
        if (meshObject.sourceGeometryName != geometryName) {
            continue;
        }
        if (meshObject.name == baseName) {
            baseNameExists = true;
            continue;
        }
        if (!meshObject.name.startsWith(baseName + "_")) {
            continue;
        }

        bool ok = false;
        const int suffix = meshObject.name.mid(baseName.size() + 1).toInt(&ok);
        if (ok) {
            maxSuffix = std::max(maxSuffix, suffix);
        }
    }

    if (!baseNameExists && maxSuffix == 0) {
        return baseName;
    }
    return QString("%1_%2").arg(baseName).arg(maxSuffix + 1, 3, 10, QChar('0'));
}

QString absoluteProjectPath(const ProjectModel &projectModel, const QString &path)
{
    return QFileInfo(path).isAbsolute()
        ? path
        : QDir(projectModel.project().rootPath).filePath(path);
}

const MeshObject *latestMeshForGeometry(const ProjectModel &projectModel, const QString &geometryName)
{
    const MeshObject *latestMesh = nullptr;
    for (const MeshObject &meshObject : projectModel.meshRepository().meshObjects()) {
        if (meshObject.sourceGeometryName != geometryName) {
            continue;
        }
        if (!latestMesh
                || meshObject.createdAt > latestMesh->createdAt
                || (meshObject.createdAt == latestMesh->createdAt && meshObject.name > latestMesh->name)) {
            latestMesh = &meshObject;
        }
    }
    return latestMesh;
}

bool readMeshDataForMeshObject(
    const ProjectModel &projectModel,
    const MeshObject &meshObject,
    MeshData &meshData,
    QString &meshAbsPath,
    MeshWorkflowResult &result
)
{
    meshAbsPath = absoluteProjectPath(projectModel, meshObject.mshFile);
    result.logMessages.append(zh(u8"当前读取网格：") + meshObject.name);
    result.logMessages.append(zh(u8"MSH 路径：") + meshAbsPath);

    if (meshObject.mshFile.trimmed().isEmpty() || !QFileInfo::exists(meshAbsPath)) {
        result.logMessages.append(zh(u8"MSH 文件不存在。"));
        return false;
    }

    meshData = MeshData{};
    meshData.name = meshObject.name;
    meshData.sourceGeometryName = meshObject.sourceGeometryName;
    meshData.mshFilePath = meshAbsPath;

    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        result.logMessages.append(zh(u8"无法读取 MSH：") + errorMessage);
        return false;
    }
    meshData.name = meshObject.name;
    meshData.sourceGeometryName = meshObject.sourceGeometryName;
    meshData.mshFilePath = meshAbsPath;
    return true;
}

void appendGmshRunLog(
    MeshWorkflowResult &workflowResult,
    const GmshRunner &gmshRunner,
    const GmshRunResult &gmshResult
)
{
    workflowResult.logMessages.append(zh(u8"Gmsh 退出码：%1").arg(gmshResult.exitCode));
    if (!gmshResult.standardOutput.trimmed().isEmpty()) {
        workflowResult.logMessages.append(zh(u8"Gmsh 标准输出：") + gmshResult.standardOutput.trimmed());
    }
    if (!gmshResult.standardError.trimmed().isEmpty()) {
        workflowResult.logMessages.append(zh(u8"Gmsh 错误输出：") + gmshResult.standardError.trimmed());
    }
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
    QString meshRelativePath = QDir("mesh").filePath(makeSafeFileBaseName(geometry.name) + ".msh");
    const QVector<MeshObject> &meshObjects = projectModel.meshRepository().meshObjects();
    for (auto it = meshObjects.crbegin(); it != meshObjects.crend(); ++it) {
        if (it->sourceGeometryName == geometry.name && !it->mshFile.trimmed().isEmpty()) {
            meshRelativePath = it->mshFile;
            break;
        }
    }
    meshAbsPath = absoluteProjectPath(projectModel, meshRelativePath);

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
            suffix += QString(" -clmin %1").arg(
                UnitConverter::lengthToMm(meshSetup.minimumSize, meshSetup.meshSizeUnit),
                0,
                'g',
                12
            );
        }
        if (meshSetup.maximumSize > 0.0) {
            suffix += QString(" -clmax %1").arg(
                UnitConverter::lengthToMm(meshSetup.maximumSize, meshSetup.meshSizeUnit),
                0,
                'g',
                12
            );
        }
    }
    const int algorithmValue = gmshOptionValue(meshSetup.algorithm);
    if (algorithmValue > 0) {
        suffix += QString(" -setnumber Mesh.Algorithm3D %1").arg(algorithmValue);
    }
    return suffix;
}

QStringList localMeshControlTexts(const ProjectModel &projectModel, const QString &geometryName)
{
    QStringList controls;
    for (const FaceGroup &faceGroup : projectModel.solverRepository().faceGroups()) {
        if (faceGroup.geometryName != geometryName || !faceGroup.localMeshEnabled || faceGroup.localMeshSize <= 0.0) {
            continue;
        }
        controls.append(
            QString("%1: size=%2, faces=%3")
                .arg(FaceGroups::displayName(faceGroup))
                .arg(faceGroup.localMeshSize, 0, 'g', 12)
                .arg(static_cast<int>(faceGroup.faceIndices.size()))
        );
    }
    return controls;
}

void appendLocalMeshRefinementLog(
    const ProjectModel &projectModel,
    const QString &geometryName,
    MeshWorkflowResult &workflowResult
)
{
    for (const FaceGroup &faceGroup : projectModel.solverRepository().faceGroups()) {
        if (faceGroup.geometryName != geometryName || !faceGroup.localMeshEnabled || faceGroup.localMeshSize <= 0.0) {
            continue;
        }
        const QString displayName = FaceGroups::displayName(faceGroup);
        const int faceCount = static_cast<int>(faceGroup.faceIndices.size());
        if (faceGroup.faceIndices.empty()) {
            workflowResult.logMessages.append(
                QString("Local mesh refinement skipped: %1 has no faces.").arg(displayName)
            );
            continue;
        }
        workflowResult.logMessages.append(
            QString("Local mesh refinement: %1, size=%2, faces=%3")
                .arg(displayName)
                .arg(faceGroup.localMeshSize, 0, 'g', 12)
                .arg(faceCount)
        );
    }
}

void applyQualityReport(MeshObject &meshObject, const MeshQualityReport &qualityReport)
{
    MeshQualityService::applyReport(meshObject, qualityReport);
}

void appendQualityLog(MeshWorkflowResult &workflowResult, const MeshQualityReport &qualityReport)
{
    workflowResult.logMessages.append(MeshQualityService::logMessages(qualityReport));
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

MeshWorkflowResult MeshWorkflowController::setLocalMeshSize(
    ProjectModel &projectModel,
    const QString &faceGroupId,
    double localMeshSize
) const
{
    MeshWorkflowResult workflowResult;
    const FaceGroupServiceResult serviceResult =
        FaceGroupService::setLocalMeshSize(projectModel, faceGroupId, localMeshSize);
    workflowResult.logMessages.append(serviceResult.logMessages);
    if (!serviceResult.success) {
        return workflowResult;
    }

    workflowResult.faceGroupTreeChanged = true;
    workflowResult.meshTreeChanged = true;
    workflowResult.resultTreeChanged = true;
    workflowResult.simulationCaseChanged = true;
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

    const QString meshName = nextMeshNameForGeometry(projectModel, geometry.name);
    const QString safeMeshName = makeSafeFileBaseName(meshName);
    const QString meshRelativePath = QDir("mesh").filePath(safeMeshName + ".msh");
    const QString meshAbsPath = QDir(projectModel.project().rootPath).filePath(meshRelativePath);

    const GmshCaseWriter gmshCaseWriter;
    const GmshCaseWriterResult gmshCaseResult = gmshCaseWriter.prepareFaceGroupExport(projectModel, geometry);
    appendLocalMeshRefinementLog(projectModel, geometry.name, workflowResult);
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
    const QString meshSizeUnit = meshSetup.meshSizeUnit.trimmed().isEmpty()
        ? QStringLiteral("mm")
        : meshSetup.meshSizeUnit.trimmed();

    workflowResult.logMessages.append(zh(u8"几何名称：") + geometry.name);
    workflowResult.logMessages.append(zh(u8"几何类型：") + geometry.type);
    workflowResult.logMessages.append(zh(u8"网格单元类型：") + displayName(meshSetup.elementType));
    workflowResult.logMessages.append(zh(u8"Gmsh 3D 算法：") + displayName(meshSetup.algorithm));
    workflowResult.logMessages.append(zh(u8"网格尺寸单位：") + meshSizeUnit + zh(u8"，传递给 Gmsh 前统一转换为 mm。"));
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
    meshObject.name = meshName;
    meshObject.sourceGeometryName = geometry.name;
    meshObject.sourceGeometryType = geometry.type;
    meshObject.sourceStepFile = geometry.stepFile;
    meshObject.mshFile = meshRelativePath;
    meshObject.type = toString(meshSetup.elementType);
    meshObject.nodeCount = meshData.nodeCount();
    meshObject.tetraCount = meshData.tetraCount();
    meshObject.tetra4Count = meshData.tetra4Count();
    meshObject.tetra10Count = meshData.tetra10Count();
    meshObject.surfaceTriangleCount = meshData.surfaceTriangleCount();
    meshObject.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    meshObject.stale = false;
    meshObject.staleReason.clear();
    meshObject.meshAutoSize = meshSetup.autoSize;
    meshObject.meshMinimumSize = meshSetup.minimumSize;
    meshObject.meshMaximumSize = meshSetup.maximumSize;
    meshObject.meshSizeUnit = meshSizeUnit;
    meshObject.meshAlgorithm = toString(meshSetup.algorithm);
    meshObject.localMeshControls = localMeshControlTexts(projectModel, geometry.name);

    const MeshQualityReport qualityReport = MeshQualityChecker::check(meshData);
    applyQualityReport(meshObject, qualityReport);

    const QVector<MeshBoundary> meshBoundaries = MeshBoundaryBuilder::build(meshData, meshObject);

    MeshManager meshManager(projectModel.project().rootPath);
    if (!meshManager.saveMeshObject(meshObject, &errorMessage)) {
        workflowResult.logMessages.append(zh(u8"保存网格对象失败：") + errorMessage);
        return workflowResult;
    }

    QVector<MeshObject> &meshObjects = projectModel.meshRepository().meshObjects();
    meshObjects.append(meshObject);
    projectModel.meshRepository().replaceMeshBoundariesForMesh(meshObject.name, meshBoundaries);
    projectModel.setSelection(Selection::item(SelectionKind::Mesh, meshObject.name, meshObject.name));

    workflowResult.meshTreeChanged = true;
    workflowResult.simulationCaseChanged = true;
    workflowResult.logMessages.append(zh(u8"网格对象已保存：mesh/") + safeMeshName + ".json");
    workflowResult.logMessages.append(meshObject.meshAutoSize
        ? zh(u8"网格尺寸参数：自动尺寸")
        : zh(u8"网格尺寸参数：手动 min=%1，max=%2")
            .arg(meshObject.meshMinimumSize, 0, 'g', 12)
            .arg(meshObject.meshMaximumSize, 0, 'g', 12));
    for (const QString &control : meshObject.localMeshControls) {
        workflowResult.logMessages.append(zh(u8"局部网格尺寸：") + control);
    }
    appendQualityLog(workflowResult, qualityReport);
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

MeshWorkflowResult MeshWorkflowController::readMeshInfo(ProjectModel &projectModel) const
{
    MeshWorkflowResult workflowResult;
    if (!projectModel.hasProject()) {
        workflowResult.logMessages.append(zh(u8"读取网格失败：请先创建或打开工程。"));
        return workflowResult;
    }

    MeshObject *targetMesh = nullptr;
    if (projectModel.selection().kind == SelectionKind::Mesh) {
        for (MeshObject &meshObject : projectModel.meshObjects()) {
            if (meshObject.name == projectModel.selection().id) {
                targetMesh = &meshObject;
                break;
            }
        }
        if (!targetMesh) {
            workflowResult.logMessages.append(zh(u8"读取网格失败：当前选择的网格对象不存在：") + projectModel.selection().id);
            return workflowResult;
        }
    } else if (const GeometryObject *selectedGeometry = projectModel.geometryForSelection()) {
        const MeshObject *latestMesh = latestMeshForGeometry(projectModel, selectedGeometry->name);
        if (!latestMesh) {
            workflowResult.logMessages.append(zh(u8"读取网格失败：当前几何没有关联的网格：") + selectedGeometry->name);
            return workflowResult;
        }
        for (MeshObject &meshObject : projectModel.meshObjects()) {
            if (meshObject.name == latestMesh->name) {
                targetMesh = &meshObject;
                break;
            }
        }
        workflowResult.logMessages.append(
            zh(u8"当前选择是几何体，已选择该几何下最新网格进行质量更新：") + latestMesh->name
        );
    } else {
        workflowResult.logMessages.append(zh(u8"读取网格失败：请先选择一个网格对象；或选择已有网格的几何体。"));
        return workflowResult;
    }

    if (!targetMesh) {
        workflowResult.logMessages.append(zh(u8"读取网格失败：无法确定要更新的网格对象。"));
        return workflowResult;
    }

    MeshData meshData;
    QString meshAbsPath;
    if (!readMeshDataForMeshObject(projectModel, *targetMesh, meshData, meshAbsPath, workflowResult)) {
        if (!workflowResult.logMessages.isEmpty()) {
            workflowResult.logMessages.last().prepend(zh(u8"读取网格失败："));
        }
        return workflowResult;
    }

    workflowResult.logMessages.append(zh(u8"读取网格成功。"));
    workflowResult.logMessages.append(zh(u8"节点数：%1").arg(meshData.nodeCount()));
    workflowResult.logMessages.append(zh(u8"四面体数：%1").arg(meshData.tetraCount()));
    const MeshQualityReport qualityReport = MeshQualityChecker::check(meshData);
    appendQualityLog(workflowResult, qualityReport);

    applyQualityReport(*targetMesh, qualityReport);
    targetMesh->nodeCount = meshData.nodeCount();
    targetMesh->tetraCount = meshData.tetraCount();
    targetMesh->tetra4Count = meshData.tetra4Count();
    targetMesh->tetra10Count = meshData.tetra10Count();
    targetMesh->surfaceTriangleCount = meshData.surfaceTriangleCount();

    QString saveError;
    if (!MeshManager(projectModel.project().rootPath).saveMeshObject(*targetMesh, &saveError)) {
        workflowResult.logMessages.append(zh(u8"保存网格质量结果失败：") + saveError);
    } else {
        workflowResult.meshTreeChanged = true;
        workflowResult.logMessages.append(zh(u8"质量结果已写入：") + targetMesh->name);
    }
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
