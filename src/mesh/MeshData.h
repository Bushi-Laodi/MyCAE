#pragma once

#include <QString>

#include <vector>

struct MeshNode
{
    int id = 0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct TetraElement
{
    int id = 0;
    int physicalGroupTag = -1;
    int elementaryEntityTag = -1;
    int node1 = 0;
    int node2 = 0;
    int node3 = 0;
    int node4 = 0;
};

struct Tetra10Element
{
    int id = 0;
    int physicalGroupTag = -1;
    int elementaryEntityTag = -1;
    int node1 = 0;
    int node2 = 0;
    int node3 = 0;
    int node4 = 0;
    int node5 = 0;
    int node6 = 0;
    int node7 = 0;
    int node8 = 0;
    int node9 = 0;
    int node10 = 0;
};

struct SurfaceTriangleElement
{
    int id = 0;
    int physicalGroupTag = -1;
    int elementaryEntityTag = -1;
    int node1 = 0;
    int node2 = 0;
    int node3 = 0;
    int node4 = 0;
    int node5 = 0;
    int node6 = 0;
};

struct MeshPhysicalGroup
{
    int dimension = 0;
    int tag = -1;
    QString name;
};

struct MeshData
{
    QString name;
    QString sourceGeometryName;
    QString mshFilePath;
    std::vector<MeshNode> nodes;
    std::vector<TetraElement> tetraElements;
    std::vector<Tetra10Element> tetra10Elements;
    std::vector<SurfaceTriangleElement> surfaceTriangles;
    std::vector<MeshPhysicalGroup> physicalGroups;

    int nodeCount() const
    {
        return static_cast<int>(nodes.size());
    }

    int tetraCount() const
    {
        return static_cast<int>(tetraElements.size() + tetra10Elements.size());
    }

    int tetra4Count() const
    {
        return static_cast<int>(tetraElements.size());
    }

    int tetra10Count() const
    {
        return static_cast<int>(tetra10Elements.size());
    }

    int surfaceTriangleCount() const
    {
        return static_cast<int>(surfaceTriangles.size());
    }

    bool isEmpty() const
    {
        return nodes.empty() && tetraElements.empty() && tetra10Elements.empty();
    }
};
