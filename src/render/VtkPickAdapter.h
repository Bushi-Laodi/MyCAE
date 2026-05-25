#pragma once

#include "picking/PickSelection.h"

#include <QString>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkPolyData;
class vtkRenderer;

class VtkPickAdapter
{
public:
    static bool pickFace(
        vtkRenderer *renderer,
        vtkActor *actor,
        vtkPolyData *polyData,
        const QString &geometryName,
        int x,
        int y,
        PickSelection &selection
    );
};
