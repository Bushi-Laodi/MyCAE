#pragma once

#include "mesh/MeshData.h"
#include "solver/calculix/CalculiXDeckTypes.h"

#include <array>
#include <map>
#include <vector>

class CalculiXSurfaceMapper
{
public:
    explicit CalculiXSurfaceMapper(const MeshData &meshData);

    std::vector<int> nodeSetForPhysicalTag(int physicalTag) const;
    std::vector<int> nodeSetForPhysicalTags(const std::vector<int> &physicalTags) const;
    std::vector<CalculiXElementSurfaceFace> surfaceFacesForPhysicalTag(int physicalTag) const;
    std::vector<CalculiXElementSurfaceFace> surfaceFacesForPhysicalTags(const std::vector<int> &physicalTags) const;

private:
    static bool containsTag(const std::vector<int> &physicalTags, int physicalTag);
    static std::array<int, 3> sortedFaceKey(int node1, int node2, int node3);
    void addFace(int elementId, const QString &faceLabel, int node1, int node2, int node3);

    const MeshData &m_meshData;
    std::map<std::array<int, 3>, CalculiXElementSurfaceFace> m_tetraFaceIndex;
};
