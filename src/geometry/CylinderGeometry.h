#pragma once

#include <QString>

struct CylinderGeometry
{
    QString name;
    double centerX = 0.0;
    double centerY = 0.0;
    double centerZ = 0.0;
    double radius = 50.0;
    double height = 200.0;
    QString unit = "mm";
    QString filePath;
    QString occBrepFile;
    QString occStepFile;
    bool occBrepSaved = false;
    bool occStepSaved = false;
    QString occBrepErrorMessage;
    QString occStepErrorMessage;
};
