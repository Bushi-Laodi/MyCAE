#pragma once

#include "mesh/MeshData.h"

#include <QString>
#include <vtkSmartPointer.h>

class vtkUnstructuredGrid;

class MeshToVtkConverter
{
public:
    static vtkSmartPointer<vtkUnstructuredGrid> toUnstructuredGrid(
        const MeshData &meshData,
        QString *errorMessage = nullptr
    );
};
