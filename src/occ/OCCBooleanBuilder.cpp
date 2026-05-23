#include "OCCBooleanBuilder.h"

#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>

namespace
{
template <typename Operation>
bool runBooleanOperation(Operation &operation, TopoDS_Shape *resultShape, QString *errorMessage)
{
    operation.Build();
    if (!operation.IsDone()) {
        if (errorMessage) {
            *errorMessage = "Open CASCADE boolean operation did not finish successfully.";
        }
        return false;
    }

    const TopoDS_Shape shape = operation.Shape();
    if (shape.IsNull()) {
        if (errorMessage) {
            *errorMessage = "Open CASCADE boolean operation produced a null shape.";
        }
        return false;
    }

    BRepCheck_Analyzer analyzer(shape);
    if (!analyzer.IsValid()) {
        if (errorMessage) {
            *errorMessage = "Open CASCADE boolean operation produced an invalid shape.";
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
            return runBooleanOperation(operation, resultShape, errorMessage);
        }
        case GeometryBooleanOperationType::Cut: {
            BRepAlgoAPI_Cut operation(leftShape, rightShape);
            return runBooleanOperation(operation, resultShape, errorMessage);
        }
        case GeometryBooleanOperationType::Common: {
            BRepAlgoAPI_Common operation(leftShape, rightShape);
            return runBooleanOperation(operation, resultShape, errorMessage);
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
