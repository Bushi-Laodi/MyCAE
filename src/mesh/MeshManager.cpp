#include "MeshManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

namespace
{
QString geometryFileStem(const QString &geometryName)
{
    QString result = geometryName.toLower();
    for (QChar &ch : result) {
        if (ch.isSpace()) {
            ch = '_';
        } else if (!ch.isLetterOrNumber() && ch != '_') {
            ch = '_';
        }
    }
    return result.isEmpty() ? QString("geometry") : result;
}

QString meshFileStem(const QString &meshName)
{
    QString result = geometryFileStem(meshName);
    return result.isEmpty() ? QString("mesh") : result;
}

QJsonArray stringListToJson(const QStringList &values)
{
    QJsonArray array;
    for (const QString &value : values) {
        array.append(value);
    }
    return array;
}

QStringList stringListFromJson(const QJsonValue &value)
{
    QStringList values;
    if (!value.isArray()) {
        return values;
    }

    const QJsonArray array = value.toArray();
    for (const QJsonValue &item : array) {
        values.append(item.toString());
    }
    return values;
}
}

MeshManager::MeshManager(const QString &projectRootPath)
    : m_projectRootPath(projectRootPath)
{
}

bool MeshManager::saveMeshObject(const MeshObject &meshObject, QString *errorMessage) const
{
    const QString meshDirPath = meshDirectory();
    if (!QDir().mkpath(meshDirPath)) {
        if (errorMessage) {
            *errorMessage = "Failed to create mesh directory: " + meshDirPath;
        }
        return false;
    }

    QJsonObject object;
    object.insert("name", meshObject.name);
    object.insert("type", meshObject.type);
    object.insert("sourceGeometry", meshObject.sourceGeometryName);
    object.insert("sourceGeometryType", meshObject.sourceGeometryType);
    object.insert("sourceStepFile", meshObject.sourceStepFile);
    object.insert("mshFile", meshObject.mshFile);
    object.insert("nodeCount", meshObject.nodeCount);
    object.insert("tetraCount", meshObject.tetraCount);
    object.insert("tetra4Count", meshObject.tetra4Count);
    object.insert("tetra10Count", meshObject.tetra10Count);
    object.insert("surfaceTriangleCount", meshObject.surfaceTriangleCount);
    object.insert("createdAt", meshObject.createdAt);
    object.insert("stale", meshObject.stale);
    object.insert("staleReason", meshObject.staleReason);

    QJsonObject sizeObject;
    sizeObject.insert("autoSize", meshObject.meshAutoSize);
    sizeObject.insert("minimumSize", meshObject.meshMinimumSize);
    sizeObject.insert("maximumSize", meshObject.meshMaximumSize);
    sizeObject.insert("algorithm", meshObject.meshAlgorithm);
    sizeObject.insert("localControls", stringListToJson(meshObject.localMeshControls));
    object.insert("meshSize", sizeObject);

    QJsonObject qualityObject;
    qualityObject.insert("checked", meshObject.qualityChecked);
    qualityObject.insert("status", meshObject.qualityStatus);
    qualityObject.insert("minimumEdgeLength", meshObject.minimumEdgeLength);
    qualityObject.insert("maximumEdgeLength", meshObject.maximumEdgeLength);
    qualityObject.insert("averageEdgeLength", meshObject.averageEdgeLength);
    qualityObject.insert("minimumTetraVolume", meshObject.minimumTetraVolume);
    qualityObject.insert("maximumTetraVolume", meshObject.maximumTetraVolume);
    qualityObject.insert("averageTetraVolume", meshObject.averageTetraVolume);
    qualityObject.insert("maximumAspectRatio", meshObject.maximumAspectRatio);
    qualityObject.insert("averageAspectRatio", meshObject.averageAspectRatio);
    qualityObject.insert("invalidTetraCount", meshObject.invalidTetraCount);
    qualityObject.insert("degenerateTetraCount", meshObject.degenerateTetraCount);
    qualityObject.insert("highAspectRatioTetraCount", meshObject.highAspectRatioTetraCount);
    qualityObject.insert("warnings", stringListToJson(meshObject.qualityWarnings));
    qualityObject.insert("invalidElementIds", stringListToJson(meshObject.invalidElementIds));
    qualityObject.insert("degenerateElementIds", stringListToJson(meshObject.degenerateElementIds));
    qualityObject.insert("highAspectRatioElementIds", stringListToJson(meshObject.highAspectRatioElementIds));
    object.insert("quality", qualityObject);

    QFile file(meshJsonPathForMeshName(meshObject.name));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write mesh object file: " + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}

bool MeshManager::loadMeshObjects(std::vector<MeshObject> &meshObjects, QString *errorMessage) const
{
    meshObjects.clear();

    const QDir meshDir(meshDirectory());
    if (!meshDir.exists()) {
        return true;
    }

    const QFileInfoList files = meshDir.entryInfoList(QStringList{"*.json"}, QDir::Files, QDir::Name);
    for (const QFileInfo &fileInfo : files) {
        MeshObject meshObject;
        if (!readMeshObject(fileInfo.absoluteFilePath(), meshObject, errorMessage)) {
            return false;
        }
        meshObjects.push_back(meshObject);
    }

    return true;
}

QString MeshManager::meshJsonPathForGeometry(const QString &geometryName) const
{
    return QDir(meshDirectory()).filePath(geometryFileStem(geometryName) + "_mesh.json");
}

QString MeshManager::meshJsonPathForMeshName(const QString &meshName) const
{
    return QDir(meshDirectory()).filePath(meshFileStem(meshName) + ".json");
}

QString MeshManager::meshMshPathForGeometry(const QString &geometryName) const
{
    return QDir(meshDirectory()).filePath(geometryFileStem(geometryName) + ".msh");
}

QString MeshManager::meshDirectory() const
{
    return QDir(m_projectRootPath).filePath("mesh");
}

bool MeshManager::readMeshObject(const QString &filePath, MeshObject &meshObject, QString *errorMessage) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "Failed to open mesh object file: " + file.errorString();
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "Invalid mesh object JSON: root is not an object.";
        }
        return false;
    }

    const QJsonObject object = document.object();
    meshObject.name = object.value("name").toString(QFileInfo(filePath).completeBaseName());
    meshObject.type = object.value("type").toString("tetra4");
    meshObject.sourceGeometryName = object.value("sourceGeometry").toString();
    meshObject.sourceGeometryType = object.value("sourceGeometryType").toString();
    meshObject.sourceStepFile = object.value("sourceStepFile").toString();
    meshObject.mshFile = object.value("mshFile").toString();
    meshObject.nodeCount = object.value("nodeCount").toInt();
    meshObject.tetraCount = object.value("tetraCount").toInt();
    meshObject.tetra4Count = object.value("tetra4Count").toInt(0);
    meshObject.tetra10Count = object.value("tetra10Count").toInt(0);
    meshObject.surfaceTriangleCount = object.value("surfaceTriangleCount").toInt(0);
    meshObject.createdAt = object.value("createdAt").toString();
    meshObject.stale = object.value("stale").toBool(false);
    meshObject.staleReason = object.value("staleReason").toString();

    const QJsonObject sizeObject = object.value("meshSize").toObject();
    meshObject.meshAutoSize = sizeObject.value("autoSize").toBool(true);
    meshObject.meshMinimumSize = sizeObject.value("minimumSize").toDouble();
    meshObject.meshMaximumSize = sizeObject.value("maximumSize").toDouble();
    meshObject.meshAlgorithm = sizeObject.value("algorithm").toString("default");
    meshObject.localMeshControls = stringListFromJson(sizeObject.value("localControls"));

    const QJsonObject qualityObject = object.value("quality").toObject();
    meshObject.qualityChecked = qualityObject.value("checked").toBool(false);
    meshObject.qualityStatus = qualityObject.value("status").toString();
    meshObject.minimumEdgeLength = qualityObject.value("minimumEdgeLength").toDouble();
    meshObject.maximumEdgeLength = qualityObject.value("maximumEdgeLength").toDouble();
    meshObject.averageEdgeLength = qualityObject.value("averageEdgeLength").toDouble();
    meshObject.minimumTetraVolume = qualityObject.value("minimumTetraVolume").toDouble();
    meshObject.maximumTetraVolume = qualityObject.value("maximumTetraVolume").toDouble();
    meshObject.averageTetraVolume = qualityObject.value("averageTetraVolume").toDouble();
    meshObject.maximumAspectRatio = qualityObject.value("maximumAspectRatio").toDouble();
    meshObject.averageAspectRatio = qualityObject.value("averageAspectRatio").toDouble();
    meshObject.invalidTetraCount = qualityObject.value("invalidTetraCount").toInt();
    meshObject.degenerateTetraCount = qualityObject.value("degenerateTetraCount").toInt();
    meshObject.highAspectRatioTetraCount = qualityObject.value("highAspectRatioTetraCount").toInt();
    meshObject.qualityWarnings = stringListFromJson(qualityObject.value("warnings"));
    meshObject.invalidElementIds = stringListFromJson(qualityObject.value("invalidElementIds"));
    meshObject.degenerateElementIds = stringListFromJson(qualityObject.value("degenerateElementIds"));
    meshObject.highAspectRatioElementIds = stringListFromJson(qualityObject.value("highAspectRatioElementIds"));

    return true;
}
