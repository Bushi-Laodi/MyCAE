#pragma once

#include <vtkSmartPointer.h>

class TopoDS_Shape;
class vtkPolyData;

class OCCShapeConverter
{
public:
    vtkSmartPointer<vtkPolyData> toPolyData(const TopoDS_Shape &shape) const;
};
