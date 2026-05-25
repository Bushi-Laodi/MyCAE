#pragma once

#include <QString>

struct MeshBoundary
{
    QString id;
    QString name;
    QString meshName;
    QString sourceGeometryName;
    QString sourceFaceGroupId;
    QString physicalGroupName;
    int physicalGroupTag = -1;
    int faceCount = 0;
};
