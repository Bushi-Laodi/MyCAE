#include "OCCCylinderBuilder.h"

#include <BRepPrimAPI_MakeCylinder.hxx>

#include <stdexcept>

TopoDS_Shape OCCCylinderBuilder::createCylinder(double radius, double height) const
{
    if (radius <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("OCC cylinder dimensions must be positive.");
    }

    BRepPrimAPI_MakeCylinder cylinderMaker(radius, height);
    TopoDS_Shape shape = cylinderMaker.Shape();
    if (shape.IsNull()) {
        throw std::runtime_error("BRepPrimAPI_MakeCylinder returned a null TopoDS_Shape.");
    }

    return shape;
}
