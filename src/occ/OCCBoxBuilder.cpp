#include "OCCBoxBuilder.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <gp_Pnt.hxx>

#include <stdexcept>

TopoDS_Shape OCCBoxBuilder::createBox(
    double length,
    double width,
    double height,
    double centerX,
    double centerY,
    double centerZ
) const
{
    if (length <= 0.0 || width <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("OCC box dimensions must be positive.");
    }

    const gp_Pnt corner(centerX - length * 0.5, centerY - width * 0.5, centerZ - height * 0.5);
    BRepPrimAPI_MakeBox boxMaker(corner, length, width, height);
    TopoDS_Shape shape = boxMaker.Shape();
    if (shape.IsNull()) {
        throw std::runtime_error("BRepPrimAPI_MakeBox returned a null TopoDS_Shape.");
    }

    return shape;
}
