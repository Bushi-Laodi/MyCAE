#include "OCCBoxBuilder.h"

#include <BRepPrimAPI_MakeBox.hxx>

#include <stdexcept>

TopoDS_Shape OCCBoxBuilder::createBox(double length, double width, double height) const
{
    if (length <= 0.0 || width <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("OCC box dimensions must be positive.");
    }

    BRepPrimAPI_MakeBox boxMaker(length, width, height);
    TopoDS_Shape shape = boxMaker.Shape();
    if (shape.IsNull()) {
        throw std::runtime_error("BRepPrimAPI_MakeBox returned a null TopoDS_Shape.");
    }

    return shape;
}
