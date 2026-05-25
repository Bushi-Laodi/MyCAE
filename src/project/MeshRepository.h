#pragma once

#include "mesh/MeshBoundary.h"
#include "mesh/MeshObject.h"

#include <QVector>

class MeshRepository
{
public:
    void clear();

    QVector<MeshObject> &meshObjects();
    const QVector<MeshObject> &meshObjects() const;
    QVector<MeshBoundary> &meshBoundaries();
    const QVector<MeshBoundary> &meshBoundaries() const;
    const MeshObject *findMeshByName(const QString &name) const;
    MeshBoundary *findMeshBoundaryById(const QString &id);
    const MeshBoundary *findMeshBoundaryById(const QString &id) const;
    void replaceMeshBoundariesForMesh(const QString &meshName, const QVector<MeshBoundary> &meshBoundaries);

private:
    QVector<MeshObject> m_meshObjects;
    QVector<MeshBoundary> m_meshBoundaries;
};
