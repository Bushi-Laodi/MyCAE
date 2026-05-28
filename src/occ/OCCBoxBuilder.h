#pragma once

#include <TopoDS_Shape.hxx>

class OCCBoxBuilder
{
public:
    TopoDS_Shape createBox(
        double length,
        double width,
        double height,
        double centerX = 0.0,
        double centerY = 0.0,
        double centerZ = 0.0
    ) const;
};
