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
    ResultNodeExtreme minUx;
    ResultNodeExtreme maxUx;
    ResultNodeExtreme minUy;
    ResultNodeExtreme maxUy;
    ResultNodeExtreme minUz;
    ResultNodeExtreme maxUz;
    ResultNodeExtreme minDisplacementMagnitude;
    ResultNodeExtreme maxDisplacementMagnitude;
    ResultElementExtreme minVonMisesStress;
    ResultElementExtreme maxVonMisesStress;
    ResultExtremeMarker selectedMinimumMarker;
    ResultExtremeMarker selectedMaximumMarker;
    ResultExtremeMarker selectedMarker;
};
