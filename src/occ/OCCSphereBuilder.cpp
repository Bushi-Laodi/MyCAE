#include "OCCSphereBuilder.h"

#include <BRepPrimAPI_MakeSphere.hxx>
#include <gp_Pnt.hxx>

#include <stdexcept>

TopoDS_Shape OCCSphereBuilder::createSphere(double radius, double centerX, double centerY, double centerZ) const
{
    if (radius <= 0.0) {
        throw std::invalid_argument("OCC sphere radius must be positive.");
    }

    BRepPrimAPI_MakeSphere sphereMaker(gp_Pnt(centerX, centerY, centerZ), radius);
    TopoDS_Shape shape = sphereMaker.Shape();
    if (shape.IsNull()) {
        throw std::runtime_error("BRepPrimAPI_MakeSphere returned a null TopoDS_Shape.");
    }

    return shape;
}
