#include "project/MeshRepository.h"

void MeshRepository::clear()
{
    m_meshObjects.clear();
}

QVector<MeshObject> &MeshRepository::meshObjects()
{
    return m_meshObjects;
}

const QVector<MeshObject> &MeshRepository::meshObjects() const
{
    return m_meshObjects;
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
