#pragma once

#include "geometry/FaceGroup.h"

#include <vtkSmartPointer.h>

#include <vector>

class vtkActor;
class vtkPolyData;

struct VtkHighlightStyle
{
    double red = 1.0;
    double green = 0.76;
    double blue = 0.08;
    double edgeRed = 1.0;
    double edgeGreen = 0.95;
    double edgeBlue = 0.55;
    double opacity = 0.92;
    double lineWidth = 3.0;
};

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
    static vtkSmartPointer<vtkActor> createFaceHighlightActor(
        vtkPolyData *polyData,
        const std::vector<int> &faceIndices,
        const std::vector<FaceReference> &faceReferences,
        const VtkHighlightStyle &style
    );
};
