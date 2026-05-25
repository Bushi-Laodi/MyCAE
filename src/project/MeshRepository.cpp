#include "project/MeshRepository.h"

void MeshRepository::clear()
{
    m_meshObjects.clear();
    m_meshBoundaries.clear();
}

QVector<MeshObject> &MeshRepository::meshObjects()
{
    return m_meshObjects;
}

const QVector<MeshObject> &MeshRepository::meshObjects() const
{
    return m_meshObjects;
}

QVector<MeshBoundary> &MeshRepository::meshBoundaries()
{
    return m_meshBoundaries;
}

const QVector<MeshBoundary> &MeshRepository::meshBoundaries() const
{
    return m_meshBoundaries;
}

const MeshObject *MeshRepository::findMeshByName(const QString &name) const
{
    for (const MeshObject &meshObject : m_meshObjects) {
        if (meshObject.name == name) {
            return &meshObject;
        }
    }
    return nullptr;
}

MeshBoundary *MeshRepository::findMeshBoundaryById(const QString &id)
{
    for (MeshBoundary &meshBoundary : m_meshBoundaries) {
        if (meshBoundary.id == id) {
            return &meshBoundary;
        }
    }
    return nullptr;
}

const MeshBoundary *MeshRepository::findMeshBoundaryById(const QString &id) const
{
    for (const MeshBoundary &meshBoundary : m_meshBoundaries) {
        if (meshBoundary.id == id) {
            return &meshBoundary;
        }
    }
    return nullptr;
}

void MeshRepository::replaceMeshBoundariesForMesh(
    const QString &meshName,
    const QVector<MeshBoundary> &meshBoundaries
)
{
    for (auto it = m_meshBoundaries.begin(); it != m_meshBoundaries.end();) {
        if (it->meshName == meshName) {
            it = m_meshBoundaries.erase(it);
        } else {
            ++it;
        }
    }

    for (const MeshBoundary &meshBoundary : meshBoundaries) {
        m_meshBoundaries.append(meshBoundary);
    }
}
