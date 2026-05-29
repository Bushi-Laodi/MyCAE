#pragma once

#include "solver/BoundaryCondition.h"

#include <QString>

#include <vector>

struct CalculiXElementSurfaceFace
{
    int elementId = 0;
    QString faceLabel;
};

struct CalculiXBoundaryExport
{
    QString boundaryId;
    QString boundaryName;
    QString nodeSetName;
    QString elementSetName;
    QString surfaceName;
    std::vector<int> nodeIds;
    std::vector<int> elementIds;
    std::vector<CalculiXElementSurfaceFace> surfaceFaces;
    bool writesFixedConstraint = false;
    bool writesDisplacementConstraint = false;
    DisplacementConstraint displacement;
};
