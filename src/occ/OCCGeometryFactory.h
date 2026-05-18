#pragma once

#include <TopoDS_Shape.hxx>

struct BoxGeometry;

class OCCGeometryFactory
{
public:
    TopoDS_Shape createShape(const BoxGeometry &box) const;
};
