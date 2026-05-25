#include "mesh/GmshCaseWriter.h"

#include "geometry/FaceGroup.h"
#include "geometry/GeometryObject.h"
#include "project/ProjectModel.h"
#include "solver/BoundaryCondition.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QSet>
#include <QTextStream>

namespace
{
QString absoluteProjectPath(const ProjectModel &projectModel, const QString &path)
{
    return QFileInfo(path).isAbsolute()
        ? path
        : QDir(projectModel.project().rootPath).filePath(path);
}

QString geoString(QString path)
{
    path.replace('\\', '/');
    path.replace('"', "\\\"");
    return path;
}

QString safePhysicalName(QString value, const QString &fallback)
{
    value = value.trimmed();
    if (value.isEmpty()) {
        value = fallback;
    }

    QString result;
    result.reserve(value.size());
    for (const QChar ch : value) {
        if (ch.isLetterOrNumber() || ch == '_' || ch == '-' || ch == '.') {
            result.append(ch);
        } else if (ch.isSpace()) {
            result.append('_');
        }
    }
    return result.isEmpty() ? fallback : result;
}

QString faceIndexList(const std::vector<int> &faceIndices)
{
    QStringList values;
    for (const int faceIndex : faceIndices) {
        if (faceIndex > 0) {
            values.append(QString::number(faceIndex));
        }
    }
    return values.join(", ");
}

QSet<QString> referencedFaceGroupIds(const ProjectModel &projectModel, const QString &geometryName)
{
    QSet<QString> ids;
    for (const BoundaryCondition &boundaryCondition : projectModel.solverRepository().boundaryConditions()) {
        if (!boundaryCondition.enabled || boundaryCondition.target.geometryName != geometryName) {
            continue;
        }
        if (!boundaryCondition.target.faceGroupId.trimmed().isEmpty()) {
            ids.insert(boundaryCondition.target.faceGroupId.trimmed());
        }
    }
    return ids;
}

bool shouldExportFaceGroup(const FaceGroup &faceGroup, const QSet<QString> &referencedIds)
{
    if (faceGroup.faceIndices.empty()) {
        return false;
    }
    return faceGroup.physicalGroupEnabled
        || faceGroup.localMeshEnabled
        || referencedIds.contains(faceGroup.id);
}

QString geoFilePathForGeometry(const ProjectModel &projectModel, const GeometryObject &geometry)
{
    const QString stepPath = absoluteProjectPath(projectModel, geometry.stepFile);
    const QFileInfo stepInfo(stepPath);
    return QDir(stepInfo.absolutePath()).filePath(stepInfo.completeBaseName() + "_physical.geo");
}

bool writeGeoFile(
    const ProjectModel &projectModel,
    const GeometryObject &geometry,
    const std::vector<GmshFaceGroupExportItem> &items,
    QString &geoFilePath,
    QString *errorMessage
)
{
    const QString stepPath = absoluteProjectPath(projectModel, geometry.stepFile);
    if (!QFileInfo::exists(stepPath)) {
        if (errorMessage) {
            *errorMessage = "STEP file does not exist: " + stepPath;
        }
        return false;
    }

    geoFilePath = geoFilePathForGeometry(projectModel, geometry);
    QFile file(geoFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write Gmsh geo file: " + file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream << "SetFactory(\"OpenCASCADE\");\n";
    stream << "Merge \"" << geoString(stepPath) << "\";\n";
    stream << "Mesh.SaveAll = 0;\n";
    stream << "v() = Volume \"*\";\n";
    stream << "Physical Volume(\"MyCAE_Domain\") = {v()};\n";

    for (const GmshFaceGroupExportItem &item : items) {
        if (!item.canMapToGmshSurface || item.faceIndices.empty()) {
            continue;
        }
        const QString physicalName = safePhysicalName(item.faceGroupId, item.displayName);
        stream << "Physical Surface(\"" << physicalName << "\") = {"
               << faceIndexList(item.faceIndices) << "};\n";
        if (item.localMeshEnabled && item.localMeshSize > 0.0) {
            stream << "Characteristic Length { PointsOf{ Surface{"
                   << faceIndexList(item.faceIndices) << "}; } } = "
                   << item.localMeshSize << ";\n";
        }
    }

    return true;
}
}

GmshCaseWriterResult GmshCaseWriter::prepareFaceGroupExport(
    const ProjectModel &projectModel,
    const GeometryObject &geometry
) const
{
    GmshCaseWriterResult result;
    result.logMessages.append("Gmsh face group export scan: " + geometry.name);
    result.meshInputFile = absoluteProjectPath(projectModel, geometry.stepFile);
    const QSet<QString> referencedIds = referencedFaceGroupIds(projectModel, geometry.name);

    for (const FaceGroup &faceGroup : projectModel.solverRepository().faceGroups()) {
        if (faceGroup.geometryName != geometry.name) {
            continue;
        }

        GmshFaceGroupExportItem item;
        item.faceGroupId = faceGroup.id;
        item.displayName = FaceGroups::displayName(faceGroup);
        item.geometryName = faceGroup.geometryName;
        item.faceIndices = faceGroup.faceIndices;
        item.physicalGroupEnabled = faceGroup.physicalGroupEnabled;
        item.localMeshEnabled = faceGroup.localMeshEnabled;
        item.localMeshSize = faceGroup.localMeshSize;
        item.canMapToGmshSurface = shouldExportFaceGroup(faceGroup, referencedIds);
        if (faceGroup.faceIndices.empty()) {
            item.reason = "face group has no picked face indices.";
        } else if (!item.canMapToGmshSurface) {
            item.reason = "physical group and local mesh export are disabled, and no boundary condition references it.";
        } else {
            item.reason = "mapped by OCC face index to Gmsh surface tag.";
        }
        result.items.push_back(item);

        if (!item.canMapToGmshSurface) {
            result.logMessages.append(
                QString("FaceGroup export skipped: %1, faces=%2, physicalGroup=%3, localMesh=%4, reason=%5")
                    .arg(item.displayName)
                    .arg(static_cast<int>(item.faceIndices.size()))
                    .arg(item.physicalGroupEnabled ? "enabled" : "disabled")
                    .arg(item.localMeshEnabled ? QString("enabled(size=%1)").arg(item.localMeshSize) : QString("disabled"))
                    .arg(item.reason)
            );
            continue;
        }

        result.hasRequestedFaceGroupExport = true;
        result.canWritePhysicalGroups = true;
        result.logMessages.append(
            QString("FaceGroup export enabled: %1, faces=%2, physicalGroup=%3, localMesh=%4, reason=%5")
                .arg(item.displayName)
                .arg(static_cast<int>(item.faceIndices.size()))
                .arg(item.physicalGroupEnabled ? "enabled" : "disabled")
                .arg(item.localMeshEnabled ? QString("enabled(size=%1)").arg(item.localMeshSize) : QString("disabled"))
                .arg(item.reason)
        );
    }

    if (!result.hasRequestedFaceGroupExport) {
        result.logMessages.append("No FaceGroup physical/local mesh export requests found.");
        return result;
    }

    QString geoError;
    QString geoPath;
    if (!writeGeoFile(projectModel, geometry, result.items, geoPath, &geoError)) {
        result.errors.append(geoError);
        return result;
    }

    result.meshInputFile = geoPath;
    result.logMessages.append("Gmsh geo file written: " + geoPath);
    result.logMessages.append("Gmsh physical surfaces use current OCC face indices as Gmsh surface tags.");
    return result;
}
