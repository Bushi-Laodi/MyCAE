#pragma once

#include <QString>

struct CylinderGeometry
{
    QString name;
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
