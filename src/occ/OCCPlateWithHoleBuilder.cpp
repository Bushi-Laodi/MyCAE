#include "occ/OCCPlateWithHoleBuilder.h"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <Precision.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include <algorithm>
#include <stdexcept>

TopoDS_Shape OCCPlateWithHoleBuilder::createPlateWithHole(
    double length,
    double width,
    double thickness,
    double holeRadius,
    double centerX,
    double centerY,
    double centerZ
) const
{
    if (length <= 0.0 || width <= 0.0 || thickness <= 0.0 || holeRadius <= 0.0) {
        throw std::invalid_argument("Plate with hole dimensions must be positive.");
    }
    if (holeRadius >= std::min(length, width) * 0.5) {
        throw std::invalid_argument("Plate hole radius must be smaller than half of the plate length and width.");
    }

    const gp_Pnt corner(centerX - length * 0.5, centerY - width * 0.5, centerZ - thickness * 0.5);
    BRepPrimAPI_MakeBox plateMaker(corner, length, width, thickness);
    const TopoDS_Shape plateShape = plateMaker.Shape();
    if (plateShape.IsNull()) {
        throw std::runtime_error("BRepPrimAPI_MakeBox returned a null plate shape.");
    }

    const double cutterHeight = thickness * 3.0;
    const gp_Pnt cutterBase(centerX, centerY, centerZ - cutterHeight * 0.5);
    BRepPrimAPI_MakeCylinder cutterMaker(gp_Ax2(cutterBase, gp_Dir(0.0, 0.0, 1.0)), holeRadius, cutterHeight);
    const TopoDS_Shape cutterShape = cutterMaker.Shape();
    if (cutterShape.IsNull()) {
        throw std::runtime_error("BRepPrimAPI_MakeCylinder returned a null hole cutter shape.");
    }

    BRepAlgoAPI_Cut cut(plateShape, cutterShape);
    cut.SetNonDestructive(true);
    cut.Build();
    if (!cut.IsDone()) {
        throw std::runtime_error("Open CASCADE boolean cut did not finish successfully.");
    }
    cut.SimplifyResult(true, true, Precision::Angular());

    const TopoDS_Shape resultShape = cut.Shape();
    if (resultShape.IsNull()) {
        throw std::runtime_error("Plate with hole boolean cut produced a null shape.");
    }
    BRepCheck_Analyzer analyzer(resultShape);
    if (!analyzer.IsValid()) {
        throw std::runtime_error("Plate with hole boolean cut produced an invalid shape.");
    }
    return resultShape;
}
