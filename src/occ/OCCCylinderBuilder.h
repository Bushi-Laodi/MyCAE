#pragma once

#include <TopoDS_Shape.hxx>

class OCCCylinderBuilder
{
public:
    TopoDS_Shape createCylinder(double radius, double height) const;
};
