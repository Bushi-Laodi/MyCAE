#pragma once

#include <QString>

#include <vector>

struct MeshBoundary
{
    QString id;
    QString name;
    QString meshName;
    QString sourceGeometryName;
    QString sourceFaceGroupId;
    QString physicalGroupName;
    int physicalGroupTag = -1;
    std::vector<int> physicalGroupTags;
    int faceCount = 0;
};
