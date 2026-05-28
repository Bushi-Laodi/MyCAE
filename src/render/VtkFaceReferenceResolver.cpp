#include "render/VtkFaceReferenceResolver.h"

#include "geometry/FaceGroup.h"
#include "picking/PickSelection.h"
#include "render/VtkFaceGeometry.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkPolyData.h>

namespace
{
constexpr const char *faceIndexArrayName = "MyCAE_FaceIndex";

std::vector<int> normalizedPositiveIndices(std::vector<int> indices)
{
    indices.erase(
        std::remove_if(indices.begin(), indices.end(), [](int value) { return value <= 0; }),
        indices.end()
    );
    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
    return indices;
}

double squaredDistance(const FacePoint &point, double x, double y, double z)
{
    const double dx = point.x - x;
    const double dy = point.y - y;
    const double dz = point.z - z;
    return dx * dx + dy * dy + dz * dz;
}

double squaredNorm(const FacePoint &point)
{
    return point.x * point.x + point.y * point.y + point.z * point.z;
}

bool hasGeometryReference(const FaceReference &reference)
{
    return squaredNorm(reference.center) > 0.0
        || squaredNorm(reference.pickedPoint) > 0.0
        || squaredNorm(reference.normal) > 0.0
        || reference.area > 0.0;
}

double boundsDiagonalSquared(vtkPolyData *polyData)
{
    double bounds[6] = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
    if (polyData) {
        polyData->GetBounds(bounds);
    }

    const double dx = bounds[1] - bounds[0];
    const double dy = bounds[3] - bounds[2];
    const double dz = bounds[5] - bounds[4];
    return std::max(1.0e-18, dx * dx + dy * dy + dz * dz);
}

double normalPenalty(const FaceReference &reference, const PickSelection &candidate)
{
    const double referenceLength = std::sqrt(squaredNorm(reference.normal));
    const double candidateLength = std::sqrt(
        candidate.normalX * candidate.normalX
        + candidate.normalY * candidate.normalY
        + candidate.normalZ * candidate.normalZ
    );
    if (referenceLength <= 1.0e-12 || candidateLength <= 1.0e-12) {
        return 0.0;
    }

    const double dot =
        (reference.normal.x * candidate.normalX
         + reference.normal.y * candidate.normalY
         + reference.normal.z * candidate.normalZ)
        / (referenceLength * candidateLength);
    return 1.0 - std::min(1.0, std::abs(dot));
}

double areaPenalty(const FaceReference &reference, const PickSelection &candidate)
{
    if (reference.area <= 0.0 || candidate.area <= 0.0) {
        return 0.0;
    }
    return std::abs(reference.area - candidate.area) / std::max(reference.area, candidate.area);
}

double candidateScore(const FaceReference &reference, const PickSelection &candidate, double diagonalSquared)
{
    double distance = std::numeric_limits<double>::max();
    if (squaredNorm(reference.center) > 0.0) {
        distance = std::min(
            distance,
            squaredDistance(reference.center, candidate.centerX, candidate.centerY, candidate.centerZ)
        );
    }
    if (squaredNorm(reference.pickedPoint) > 0.0) {
        distance = std::min(
            distance,
            squaredDistance(reference.pickedPoint, candidate.centerX, candidate.centerY, candidate.centerZ)
        );
    }
    if (distance == std::numeric_limits<double>::max()) {
        distance = 0.0;
    }

    return distance / diagonalSquared
        + normalPenalty(reference, candidate) * 0.20
        + areaPenalty(reference, candidate) * 0.05;
}

int bestFaceIndexForReference(vtkPolyData *polyData, vtkIntArray *faceIndexArray, const FaceReference &reference)
{
    const double diagonalSquared = boundsDiagonalSquared(polyData);
    double bestScore = std::numeric_limits<double>::max();
    int bestFaceIndex = reference.faceIndex;

    for (vtkIdType cellId = 0; cellId < polyData->GetNumberOfCells(); ++cellId) {
        if (cellId >= faceIndexArray->GetNumberOfTuples()) {
            break;
        }

        PickSelection candidate;
        VtkFaceGeometry::fillPickSelectionFromCell(polyData->GetCell(cellId), candidate);
        const double score = candidateScore(reference, candidate, diagonalSquared);
        if (score < bestScore) {
            bestScore = score;
            bestFaceIndex = faceIndexArray->GetValue(cellId);
        }
    }

    return bestFaceIndex;
}
}

std::vector<int> VtkFaceReferenceResolver::resolveFaceIndices(
    vtkPolyData *polyData,
    const std::vector<int> &fallbackFaceIndices,
    const std::vector<FaceReference> &faceReferences
)
{
    if (!polyData || faceReferences.empty()) {
        return normalizedPositiveIndices(fallbackFaceIndices);
    }

    auto *faceIndexArray = vtkIntArray::SafeDownCast(polyData->GetCellData()->GetArray(faceIndexArrayName));
    if (!faceIndexArray) {
        return normalizedPositiveIndices(fallbackFaceIndices);
    }

    std::vector<int> resolvedIndices;
    resolvedIndices.reserve(faceReferences.size());
    for (const FaceReference &reference : faceReferences) {
        const int resolvedIndex = hasGeometryReference(reference)
            ? bestFaceIndexForReference(polyData, faceIndexArray, reference)
            : reference.faceIndex;
        if (resolvedIndex > 0) {
            resolvedIndices.push_back(resolvedIndex);
        }
    }

    if (resolvedIndices.empty()) {
        return normalizedPositiveIndices(fallbackFaceIndices);
    }
    return normalizedPositiveIndices(std::move(resolvedIndices));
}
