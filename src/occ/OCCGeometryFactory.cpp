#include "OCCGeometryFactory.h"

#include "geometry/BoxGeometry.h"
#include "occ/OCCBoxBuilder.h"

TopoDS_Shape OCCGeometryFactory::createShape(const BoxGeometry &box) const
{
    OCCBoxBuilder builder;
    return builder.createBox(box.length, box.width, box.height);
}
