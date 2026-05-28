#pragma once

#include <vector>

class vtkPolyData;
struct FaceReference;

namespace VtkFaceReferenceResolver
{
std::vector<int> resolveFaceIndices(
    vtkPolyData *polyData,
    const std::vector<int> &fallbackFaceIndices,
    const std::vector<FaceReference> &faceReferences
);
}
