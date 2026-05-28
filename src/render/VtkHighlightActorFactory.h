#pragma once

#include "geometry/FaceGroup.h"

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
    static vtkSmartPointer<vtkActor> createFaceHighlightActor(
        vtkPolyData *polyData,
        const std::vector<int> &faceIndices,
        const std::vector<FaceReference> &faceReferences
    );
};
