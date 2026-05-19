#include "MeshManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

namespace
{
QString geometryFileStem(const QString &geometryName)
{
    return geometryName.toLower();
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
    object.insert("mshFile", meshObject.mshFile);
    object.insert("nodeCount", meshObject.nodeCount);
    object.insert("tetraCount", meshObject.tetraCount);
    object.insert("createdAt", meshObject.createdAt);

    QFile file(meshJsonPathForGeometry(meshObject.sourceGeometryName));
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

    const QFileInfoList files = meshDir.entryInfoList(QStringList{"*_mesh.json"}, QDir::Files, QDir::Name);
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
    meshObject.mshFile = object.value("mshFile").toString();
    meshObject.nodeCount = object.value("nodeCount").toInt();
    meshObject.tetraCount = object.value("tetraCount").toInt();
    meshObject.createdAt = object.value("createdAt").toString();

    return true;
}
