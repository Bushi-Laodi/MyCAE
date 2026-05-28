#include "OCCBooleanBuilder.h"

#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <Precision.hxx>
#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>

namespace
{
constexpr double BooleanFuzzyTolerance = 1.0e-6;

bool boundingBoxesOverlap(const TopoDS_Shape &leftShape, const TopoDS_Shape &rightShape, double tolerance)
{
    Bnd_Box leftBox;
    Bnd_Box rightBox;
    BRepBndLib::Add(leftShape, leftBox);
    BRepBndLib::Add(rightShape, rightBox);
    leftBox.Enlarge(tolerance);
    rightBox.Enlarge(tolerance);
    return !leftBox.IsOut(rightBox);
}

template <typename Operation>
bool runBooleanOperation(
    Operation &operation,
    TopoDS_Shape *resultShape,
    QString *errorMessage,
    const QString &operationName
)
{
    operation.SetNonDestructive(true);
    operation.SetFuzzyValue(BooleanFuzzyTolerance);
    operation.Build();
    if (!operation.IsDone()) {
        if (errorMessage) {
            *errorMessage = operationName + " did not finish successfully.";
        }
        return false;
    }

    operation.SimplifyResult(true, true, Precision::Angular());

    const TopoDS_Shape shape = operation.Shape();
    if (shape.IsNull()) {
        if (errorMessage) {
            *errorMessage = operationName + " produced a null shape.";
        }
        return false;
    }

    BRepCheck_Analyzer analyzer(shape);
    if (!analyzer.IsValid()) {
        if (errorMessage) {
            *errorMessage = operationName + " produced an invalid shape.";
        }
        return false;
    }

    *resultShape = shape;
    return true;
}
}

bool OCCBooleanBuilder::build(
    const TopoDS_Shape &leftShape,
    const TopoDS_Shape &rightShape,
    GeometryBooleanOperationType operationType,
    TopoDS_Shape *resultShape,
    QString *errorMessage
) const
{
    if (!resultShape) {
        if (errorMessage) {
            *errorMessage = "Internal error: boolean result shape output is null.";
        }
        return false;
    }
    if (leftShape.IsNull() || rightShape.IsNull()) {
        if (errorMessage) {
            *errorMessage = "Boolean operation requires two valid input shapes.";
        }
        return false;
    }

    try {
        switch (operationType) {
        case GeometryBooleanOperationType::Union: {
            BRepAlgoAPI_Fuse operation(leftShape, rightShape);
            return runBooleanOperation(operation, resultShape, errorMessage, "Open CASCADE boolean union");
        }
        case GeometryBooleanOperationType::Cut: {
            if (!boundingBoxesOverlap(leftShape, rightShape, BooleanFuzzyTolerance)) {
                if (errorMessage) {
                    *errorMessage = "Boolean cut inputs do not overlap. Move the tool body so it intersects the target body.";
                }
                return false;
            }
            BRepAlgoAPI_Cut operation(leftShape, rightShape);
            return runBooleanOperation(operation, resultShape, errorMessage, "Open CASCADE boolean cut");
        }
        case GeometryBooleanOperationType::Common: {
            if (!boundingBoxesOverlap(leftShape, rightShape, BooleanFuzzyTolerance)) {
                if (errorMessage) {
                    *errorMessage = "Boolean common inputs do not overlap. Move the bodies so their volumes intersect.";
                }
                return false;
            }
            BRepAlgoAPI_Common operation(leftShape, rightShape);
            return runBooleanOperation(operation, resultShape, errorMessage, "Open CASCADE boolean common");
        }
        }
    } catch (const Standard_Failure &failure) {
        if (errorMessage) {
            *errorMessage = QString("Open CASCADE boolean operation failed: %1").arg(failure.GetMessageString());
        }
        return false;
    }

    if (errorMessage) {
        *errorMessage = "Unsupported boolean operation type.";
    }
    return false;
}
