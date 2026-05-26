#pragma once

#include "result/ResultExtrema.h"

#include <QString>
#include <QStringList>

struct ResultObject
{
    QString id;
    QString name;
    QString solverName;
    QString meshName;
    QString casePath;
    QString logFile;
    QString datFile;
    QString frdFile;
    QString staFile;
    QStringList resultFiles;
    QStringList availableFields;
    QString primaryFieldName;
    QString displayFieldName;
    double deformationScale = 0.0;
    bool showMeshEdges = true;
    bool showUndeformedOverlay = false;
    bool resultFilesComplete = false;
    int matchedNodeCount = 0;
    int meshNodeCount = 0;
    int matchedElementCount = 0;
    int meshElementCount = 0;
    double scalarMin = 0.0;
    double scalarMax = 0.0;
    bool scalarRangeLocked = false;
    double lockedScalarMin = 0.0;
    double lockedScalarMax = 0.0;
    ResultExtrema extrema;
    QStringList checkMessages;
    QString createdAt;
    QString summary;
    bool success = false;
};
