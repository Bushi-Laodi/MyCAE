#pragma once

#include <QString>

struct MeshObject
{
    QString name;
    QString sourceGeometryName;
    QString sourceGeometryType;
    QString sourceStepFile;
    QString mshFile;
    QString type = "tetra4";
    int nodeCount = 0;
    int tetraCount = 0;
    QString createdAt;
};
