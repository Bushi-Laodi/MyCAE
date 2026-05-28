#pragma once

#include <TopoDS_Shape.hxx>

#include <QString>

struct RenderGeometryItem
{
    QString name;
    QString type;
    TopoDS_Shape shape;
};
