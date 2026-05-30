#pragma once

#include <TopoDS_Shape.hxx>

class OCCPlateWithHoleBuilder
{
public:
    TopoDS_Shape createPlateWithHole(
        double length,
        double width,
        double thickness,
        double holeRadius,
        double centerX,
        double centerY,
        double centerZ
    ) const;
};
