#include "result/ResultDataLoader.h"

#include "mesh/MeshObject.h"
#include "mesh/MshReader.h"
#include "project/ProjectModel.h"
#include "result/ResultObject.h"

#include <QDir>
#include <QFileInfo>

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

QString firstExistingDatFile(const ProjectModel &projectModel, const ResultObject &resultObject)
{
    if (!resultObject.datFile.isEmpty()) {
        const QString datFilePath = absoluteProjectPath(projectModel, resultObject.datFile);
        if (QFileInfo::exists(datFilePath)) {
            return datFilePath;
        }
    }

    const QDir caseDir(absoluteProjectPath(projectModel, resultObject.casePath));
    const QFileInfoList datFiles = caseDir.entryInfoList(QStringList{"*.dat"}, QDir::Files, QDir::Time);
    return datFiles.isEmpty() ? QString() : datFiles.front().absoluteFilePath();
}
}

ResultDataLoadResult ResultDataLoader::loadCalculiXResult(
    const ProjectModel &projectModel,
    const ResultObject &resultObject
) const
{
    ResultDataLoadResult result;
    if (!resultObject.solverName.contains("CalculiX", Qt::CaseInsensitive)) {
        result.errors.append("Only CalculiX results are supported currently.");
        return result;
    }

    const MeshObject *meshObject = resultMeshOrFallback(projectModel, resultObject);
    if (!meshObject) {
        result.errors.append("Result mesh is not loaded or cannot be inferred: " + resultObject.meshName);
        return result;
    }

    result.meshFilePath = absoluteProjectPath(projectModel, meshObject->mshFile);
    if (!QFileInfo::exists(result.meshFilePath)) {
        result.errors.append("MSH file does not exist: " + result.meshFilePath);
        return result;
    }

    result.meshData.sourceGeometryName = meshObject->sourceGeometryName;
    QString meshError;
    if (!MshReader::readMsh2(result.meshFilePath, result.meshData, &meshError)) {
        result.errors.append("Cannot read mesh: " + meshError);
        return result;
    }

    result.datFilePath = firstExistingDatFile(projectModel, resultObject);
    if (result.datFilePath.isEmpty()) {
        result.errors.append("No CalculiX .dat file found in " + absoluteProjectPath(projectModel, resultObject.casePath));
        return result;
    }

    const CalculiXDatReadResult datReadResult = CalculiXDatResultReader().read(result.datFilePath);
    result.warnings.append(datReadResult.warnings);
    result.errors.append(datReadResult.errors);
    if (!datReadResult.success) {
        return result;
    }

    result.datResult = datReadResult.result;
    result.success = true;
    return result;
}
