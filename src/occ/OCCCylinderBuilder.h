#pragma once

#include <TopoDS_Shape.hxx>

class OCCCylinderBuilder
{
public:
    TopoDS_Shape createCylinder(
        double radius,
        double height,
        double centerX = 0.0,
        double centerY = 0.0,
        double centerZ = 0.0
    ) const;
};
