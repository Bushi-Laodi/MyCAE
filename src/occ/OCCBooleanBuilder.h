#pragma once

#include "geometry/GeometryBooleanOperation.h"

#include <QString>

class TopoDS_Shape;

class OCCBooleanBuilder
{
public:
    bool build(
        const TopoDS_Shape &leftShape,
        const TopoDS_Shape &rightShape,
        GeometryBooleanOperationType operationType,
        TopoDS_Shape *resultShape,
        QString *errorMessage = nullptr
    ) const;
};
