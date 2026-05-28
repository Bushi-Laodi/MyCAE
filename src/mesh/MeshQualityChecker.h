#pragma once

#include "mesh/MeshData.h"

#include <QString>
#include <QStringList>

struct MeshQualityReport
{
    bool checked = false;
    int nodeCount = 0;
    int tetraCount = 0;
    int tetra4Count = 0;
    int tetra10Count = 0;
    int invalidTetraCount = 0;
    int degenerateTetraCount = 0;
    int highAspectRatioTetraCount = 0;
    double minimumEdgeLength = 0.0;
    double maximumEdgeLength = 0.0;
    double averageEdgeLength = 0.0;
    double minimumTetraVolume = 0.0;
    double maximumTetraVolume = 0.0;
    double averageTetraVolume = 0.0;
    double maximumAspectRatio = 0.0;
    double averageAspectRatio = 0.0;
    QString status;
    QStringList warnings;
};

class MeshQualityChecker
{
public:
    static MeshQualityReport check(const MeshData &meshData);
};
