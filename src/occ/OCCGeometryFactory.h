#pragma once

#include <TopoDS_Shape.hxx>

struct BoxGeometry;
struct CylinderGeometry;
struct PlateWithHoleGeometry;
struct SphereGeometry;

class OCCGeometryFactory
{
public:
    TopoDS_Shape createShape(const BoxGeometry &box) const;
    TopoDS_Shape createShape(const CylinderGeometry &cylinder) const;
    TopoDS_Shape createShape(const SphereGeometry &sphere) const;
    TopoDS_Shape createShape(const PlateWithHoleGeometry &plate) const;
};
