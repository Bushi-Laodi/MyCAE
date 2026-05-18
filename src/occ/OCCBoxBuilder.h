#pragma once

#include <TopoDS_Shape.hxx>

class OCCBoxBuilder
{
public:
    TopoDS_Shape createBox(double length, double width, double height) const;
};
