#pragma once

#include "mesh/MeshObject.h"

#include <QVector>

class MeshRepository
{
public:
    void clear();

    QVector<MeshObject> &meshObjects();
    const QVector<MeshObject> &meshObjects() const;
    const MeshObject *findMeshByName(const QString &name) const;

private:
    QVector<MeshObject> m_meshObjects;
};
