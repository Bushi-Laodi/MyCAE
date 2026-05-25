#pragma once

#include <QString>

struct ResultNodeExtreme
{
    bool valid = false;
    int nodeId = 0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double value = 0.0;
    QString fieldName;
};

struct ResultElementExtreme
{
    bool valid = false;
    int elementId = 0;
    // Element coordinates are the tetrahedron centroid.
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double value = 0.0;
    QString fieldName;
};

struct ResultExtremeMarker
{
    bool valid = false;
    bool element = false;
    int id = 0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double value = 0.0;
    QString fieldName;
};

struct ResultExtrema
{
    ResultNodeExtreme maxUx;
    ResultNodeExtreme maxUy;
    ResultNodeExtreme maxUz;
    ResultNodeExtreme maxDisplacementMagnitude;
    ResultElementExtreme maxVonMisesStress;
    ResultExtremeMarker selectedMarker;
};
