#pragma once

#include <QString>

struct PlateWithHoleGeometry
{
    QString name;
    double length = 200.0;
    double width = 100.0;
    double thickness = 10.0;
    double holeRadius = 20.0;
    double centerX = 0.0;
    double centerY = 0.0;
    double centerZ = 0.0;
    QString unit = "mm";
    QString filePath;
    QString occBrepFile;
    QString occStepFile;
    bool occBrepSaved = false;
    bool occStepSaved = false;
    QString occBrepErrorMessage;
    QString occStepErrorMessage;
};
