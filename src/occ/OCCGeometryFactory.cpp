#include "OCCGeometryFactory.h"

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "geometry/PlateWithHoleGeometry.h"
#include "geometry/SphereGeometry.h"
#include "occ/OCCBoxBuilder.h"
#include "occ/OCCCylinderBuilder.h"
#include "occ/OCCPlateWithHoleBuilder.h"
#include "occ/OCCSphereBuilder.h"

TopoDS_Shape OCCGeometryFactory::createShape(const BoxGeometry &box) const
{
    OCCBoxBuilder builder;
    return builder.createBox(box.length, box.width, box.height, box.centerX, box.centerY, box.centerZ);
}

TopoDS_Shape OCCGeometryFactory::createShape(const CylinderGeometry &cylinder) const
{
    OCCCylinderBuilder builder;
    return builder.createCylinder(
        cylinder.radius,
        cylinder.height,
        cylinder.centerX,
        cylinder.centerY,
        cylinder.centerZ
    );
}

TopoDS_Shape OCCGeometryFactory::createShape(const SphereGeometry &sphere) const
{
    OCCSphereBuilder builder;
    return builder.createSphere(sphere.radius, sphere.centerX, sphere.centerY, sphere.centerZ);
}

TopoDS_Shape OCCGeometryFactory::createShape(const PlateWithHoleGeometry &plate) const
{
    OCCPlateWithHoleBuilder builder;
    return builder.createPlateWithHole(
        plate.length,
        plate.width,
        plate.thickness,
        plate.holeRadius,
        plate.centerX,
        plate.centerY,
        plate.centerZ
    );
}
