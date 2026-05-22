#include "OCCGeometryFactory.h"

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "occ/OCCBoxBuilder.h"
#include "occ/OCCCylinderBuilder.h"

TopoDS_Shape OCCGeometryFactory::createShape(const BoxGeometry &box) const
{
    OCCBoxBuilder builder;
    return builder.createBox(box.length, box.width, box.height);
}

TopoDS_Shape OCCGeometryFactory::createShape(const CylinderGeometry &cylinder) const
{
    OCCCylinderBuilder builder;
    return builder.createCylinder(cylinder.radius, cylinder.height);
}
