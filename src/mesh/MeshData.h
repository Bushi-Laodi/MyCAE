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
    int node1 = 0;
    int node2 = 0;
    int node3 = 0;
    int node4 = 0;
};

struct MeshData
{
    QString name;
    QString sourceGeometryName;
    QString mshFilePath;
    std::vector<MeshNode> nodes;
    std::vector<TetraElement> tetraElements;

    int nodeCount() const
    {
        return static_cast<int>(nodes.size());
    }

    int tetraCount() const
    {
        return static_cast<int>(tetraElements.size());
    }

    bool isEmpty() const
    {
        return nodes.empty() && tetraElements.empty();
    }
};
