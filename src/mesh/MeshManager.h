#pragma once

#include "mesh/MeshObject.h"

#include <QString>

#include <vector>

class MeshManager
{
public:
    explicit MeshManager(const QString &projectRootPath);

    bool saveMeshObject(const MeshObject &meshObject, QString *errorMessage = nullptr) const;
    bool loadMeshObjects(std::vector<MeshObject> &meshObjects, QString *errorMessage = nullptr) const;

    QString meshJsonPathForGeometry(const QString &geometryName) const;
    QString meshMshPathForGeometry(const QString &geometryName) const;

private:
    QString meshDirectory() const;
    bool readMeshObject(const QString &filePath, MeshObject &meshObject, QString *errorMessage) const;

    QString m_projectRootPath;
};
