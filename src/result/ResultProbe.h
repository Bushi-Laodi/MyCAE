#pragma once

#include <QString>

struct ResultProbe
{
    bool valid = false;
    int nodeId = 0;
    int elementId = 0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double ux = 0.0;
    double uy = 0.0;
    double uz = 0.0;
    double displacementMagnitude = 0.0;
    double vonMisesStress = 0.0;
    QString source;
};
