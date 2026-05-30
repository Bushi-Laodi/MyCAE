#pragma once

#include <QString>
#include <QStringList>

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
    int tetra4Count = 0;
    int tetra10Count = 0;
    int surfaceTriangleCount = 0;
    QString createdAt;
    bool stale = false;
    QString staleReason;
    bool meshAutoSize = true;
    double meshMinimumSize = 0.0;
    double meshMaximumSize = 0.0;
    QString meshAlgorithm = "default";
    QStringList localMeshControls;
    bool qualityChecked = false;
    QString qualityStatus;
    double minimumEdgeLength = 0.0;
    double maximumEdgeLength = 0.0;
    double averageEdgeLength = 0.0;
    double minimumTetraVolume = 0.0;
    double maximumTetraVolume = 0.0;
    double averageTetraVolume = 0.0;
    double maximumAspectRatio = 0.0;
    double averageAspectRatio = 0.0;
    int invalidTetraCount = 0;
    int degenerateTetraCount = 0;
    int highAspectRatioTetraCount = 0;
    QStringList qualityWarnings;
    QStringList invalidElementIds;
    QStringList degenerateElementIds;
    QStringList highAspectRatioElementIds;
};
