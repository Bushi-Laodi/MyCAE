#pragma once

#include <vtkSmartPointer.h>

#include <vector>

class vtkActor;
class vtkPolyData;

class VtkHighlightActorFactory
{
public:
    static vtkSmartPointer<vtkActor> createFaceHighlightActor(
        vtkPolyData *polyData,
        const std::vector<int> &faceIndices
    );
};
