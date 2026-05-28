#include "OCCCylinderBuilder.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include <stdexcept>

TopoDS_Shape OCCCylinderBuilder::createCylinder(
    double radius,
    double height,
    double centerX,
    double centerY,
    double centerZ
) const
{
    if (radius <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("OCC cylinder dimensions must be positive.");
    }

    const gp_Pnt baseCenter(centerX, centerY, centerZ - height * 0.5);
    BRepPrimAPI_MakeCylinder cylinderMaker(gp_Ax2(baseCenter, gp_Dir(0.0, 0.0, 1.0)), radius, height);
    TopoDS_Shape shape = cylinderMaker.Shape();
    if (shape.IsNull()) {
        throw std::runtime_error("BRepPrimAPI_MakeCylinder returned a null TopoDS_Shape.");
    }

    return shape;
}
