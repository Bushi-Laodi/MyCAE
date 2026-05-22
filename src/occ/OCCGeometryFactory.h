#pragma once

#include <TopoDS_Shape.hxx>

struct BoxGeometry;
struct CylinderGeometry;

class OCCGeometryFactory
{
public:
    TopoDS_Shape createShape(const BoxGeometry &box) const;
    TopoDS_Shape createShape(const CylinderGeometry &cylinder) const;
};
