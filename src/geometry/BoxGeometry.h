#pragma once

#include <QString>

struct BoxGeometry
{
    QString name;
    double length = 200.0;
    double width = 200.0;
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
