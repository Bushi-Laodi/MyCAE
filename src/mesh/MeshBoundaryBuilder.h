#pragma once

#include "mesh/MeshBoundary.h"

#include <QVector>

struct MeshData;
struct MeshObject;

class MeshBoundaryBuilder
{
public:
    static QVector<MeshBoundary> build(const MeshData &meshData, const MeshObject &meshObject);
};
