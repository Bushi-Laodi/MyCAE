#include "solver/calculix/CalculiXSurfaceMapper.h"

#include <QSet>

#include <algorithm>

CalculiXSurfaceMapper::CalculiXSurfaceMapper(const MeshData &meshData)
    : m_meshData(meshData)
{
    for (const TetraElement &tetra : m_meshData.tetraElements) {
        addFace(tetra.id, "S1", tetra.node1, tetra.node2, tetra.node3);
        addFace(tetra.id, "S2", tetra.node1, tetra.node2, tetra.node4);
        addFace(tetra.id, "S3", tetra.node2, tetra.node3, tetra.node4);
        addFace(tetra.id, "S4", tetra.node1, tetra.node3, tetra.node4);
    }
    for (const Tetra10Element &tetra : m_meshData.tetra10Elements) {
        addFace(tetra.id, "S1", tetra.node1, tetra.node2, tetra.node3);
        addFace(tetra.id, "S2", tetra.node1, tetra.node2, tetra.node4);
        addFace(tetra.id, "S3", tetra.node2, tetra.node3, tetra.node4);
        addFace(tetra.id, "S4", tetra.node1, tetra.node3, tetra.node4);
    }
}

std::vector<int> CalculiXSurfaceMapper::nodeSetForPhysicalTag(int physicalTag) const
{
    return nodeSetForPhysicalTags({physicalTag});
}

std::vector<int> CalculiXSurfaceMapper::nodeSetForPhysicalTags(const std::vector<int> &physicalTags) const
{
    QSet<int> nodeIds;
    for (const SurfaceTriangleElement &triangle : m_meshData.surfaceTriangles) {
        if (!containsTag(physicalTags, triangle.physicalGroupTag)) {
            continue;
        }
        nodeIds.insert(triangle.node1);
        nodeIds.insert(triangle.node2);
        nodeIds.insert(triangle.node3);
        if (triangle.node4 > 0) {
            nodeIds.insert(triangle.node4);
        }
        if (triangle.node5 > 0) {
            nodeIds.insert(triangle.node5);
        }
        if (triangle.node6 > 0) {
            nodeIds.insert(triangle.node6);
        }
    }

    std::vector<int> nodes;
    nodes.reserve(static_cast<size_t>(nodeIds.size()));
    for (const int nodeId : nodeIds) {
        nodes.push_back(nodeId);
    }
    std::sort(nodes.begin(), nodes.end());
    return nodes;
}

std::vector<CalculiXElementSurfaceFace> CalculiXSurfaceMapper::surfaceFacesForPhysicalTag(int physicalTag) const
{
    return surfaceFacesForPhysicalTags({physicalTag});
}

std::vector<CalculiXElementSurfaceFace> CalculiXSurfaceMapper::surfaceFacesForPhysicalTags(
    const std::vector<int> &physicalTags
) const
{
    std::vector<CalculiXElementSurfaceFace> surfaceFaces;
    for (const SurfaceTriangleElement &triangle : m_meshData.surfaceTriangles) {
        if (!containsTag(physicalTags, triangle.physicalGroupTag)) {
            continue;
        }

        const auto iterator = m_tetraFaceIndex.find(sortedFaceKey(triangle.node1, triangle.node2, triangle.node3));
        if (iterator != m_tetraFaceIndex.end()) {
            surfaceFaces.push_back(iterator->second);
        }
    }
    return surfaceFaces;
}

bool CalculiXSurfaceMapper::containsTag(const std::vector<int> &physicalTags, int physicalTag)
{
    return std::find(physicalTags.begin(), physicalTags.end(), physicalTag) != physicalTags.end();
}

std::array<int, 3> CalculiXSurfaceMapper::sortedFaceKey(int node1, int node2, int node3)
{
    std::array<int, 3> key = {node1, node2, node3};
    std::sort(key.begin(), key.end());
    return key;
}

void CalculiXSurfaceMapper::addFace(
    int elementId,
    const QString &faceLabel,
    int node1,
    int node2,
    int node3
)
{
    m_tetraFaceIndex.emplace(
        sortedFaceKey(node1, node2, node3),
        CalculiXElementSurfaceFace{elementId, faceLabel}
    );
}
