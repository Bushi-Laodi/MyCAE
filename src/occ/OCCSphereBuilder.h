#pragma once

#include <TopoDS_Shape.hxx>

class OCCSphereBuilder
{
public:
    TopoDS_Shape createSphere(double radius, double centerX = 0.0, double centerY = 0.0, double centerZ = 0.0) const;
};
