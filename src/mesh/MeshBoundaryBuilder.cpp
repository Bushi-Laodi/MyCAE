#include "mesh/MeshBoundaryBuilder.h"

#include "geometry/FaceGroup.h"
#include "mesh/MeshData.h"
#include "mesh/MeshObject.h"

#include <QHash>

QVector<MeshBoundary> MeshBoundaryBuilder::build(const MeshData &meshData, const MeshObject &meshObject)
{
    QHash<int, int> triangleCountByPhysicalTag;
    for (const SurfaceTriangleElement &triangle : meshData.surfaceTriangles) {
        if (triangle.physicalGroupTag >= 0) {
            ++triangleCountByPhysicalTag[triangle.physicalGroupTag];
        }
    }

    QVector<MeshBoundary> meshBoundaries;
    for (const MeshPhysicalGroup &physicalGroup : meshData.physicalGroups) {
        if (physicalGroup.dimension != 2) {
            continue;
        }

        MeshBoundary boundary;
        boundary.id = meshObject.name + "." + physicalGroup.name;
        boundary.name = FaceGroups::nameFromId(physicalGroup.name);
        boundary.meshName = meshObject.name;
        boundary.sourceGeometryName = meshObject.sourceGeometryName;
        boundary.sourceFaceGroupId = physicalGroup.name;
        boundary.physicalGroupName = physicalGroup.name;
        boundary.physicalGroupTag = physicalGroup.tag;
        boundary.faceCount = triangleCountByPhysicalTag.value(physicalGroup.tag, 0);
        meshBoundaries.append(boundary);
    }
    return meshBoundaries;
}
