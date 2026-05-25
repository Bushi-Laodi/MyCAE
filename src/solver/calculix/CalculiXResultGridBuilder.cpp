#include "solver/calculix/CalculiXResultGridBuilder.h"

#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTetra.h>
#include <vtkUnstructuredGrid.h>

#include <cmath>
#include <limits>
#include <unordered_map>

namespace
{
double displacementMagnitude(const CalculiXNodeDisplacement &value)
{
    return std::sqrt(value.ux * value.ux + value.uy * value.uy + value.uz * value.uz);
}
}

CalculiXResultGridBuildResult CalculiXResultGridBuilder::buildDisplacementGrid(
    const MeshData &meshData,
    const CalculiXDatResult &result
) const
{
    CalculiXResultGridBuildResult buildResult;
    buildResult.scalarName = "Displacement Magnitude";
    buildResult.meshNodeCount = meshData.nodeCount();

    if (meshData.nodes.empty() || meshData.tetraElements.empty()) {
        buildResult.errors.append("Cannot build result grid: mesh has no nodes or tetrahedra.");
        return buildResult;
    }
    if (result.displacements.empty()) {
        buildResult.errors.append("Cannot build result grid: displacement field is empty.");
        return buildResult;
    }

    std::unordered_map<int, CalculiXNodeDisplacement> displacementByNodeId;
    displacementByNodeId.reserve(result.displacements.size());
    for (const CalculiXNodeDisplacement &displacement : result.displacements) {
        displacementByNodeId.insert_or_assign(displacement.nodeId, displacement);
    }

    vtkNew<vtkPoints> points;
    vtkNew<vtkDoubleArray> displacementVector;
    vtkNew<vtkDoubleArray> displacementMagnitudeArray;
    displacementVector->SetName("Displacement");
    displacementVector->SetNumberOfComponents(3);
    displacementMagnitudeArray->SetName(buildResult.scalarName.toUtf8().constData());

    std::unordered_map<int, vtkIdType> nodeIdToVtkId;
    nodeIdToVtkId.reserve(meshData.nodes.size());

    double scalarMin = std::numeric_limits<double>::max();
    double scalarMax = std::numeric_limits<double>::lowest();

    for (const MeshNode &node : meshData.nodes) {
        const vtkIdType vtkPointId = points->InsertNextPoint(node.x, node.y, node.z);
        nodeIdToVtkId.insert({node.id, vtkPointId});

        const auto displacementIt = displacementByNodeId.find(node.id);
        if (displacementIt == displacementByNodeId.end()) {
            displacementVector->InsertNextTuple3(0.0, 0.0, 0.0);
            displacementMagnitudeArray->InsertNextValue(0.0);
            scalarMin = std::min(scalarMin, 0.0);
            scalarMax = std::max(scalarMax, 0.0);
            continue;
        }

        const CalculiXNodeDisplacement &displacement = displacementIt->second;
        const double magnitude = displacementMagnitude(displacement);
        displacementVector->InsertNextTuple3(displacement.ux, displacement.uy, displacement.uz);
        displacementMagnitudeArray->InsertNextValue(magnitude);
        scalarMin = std::min(scalarMin, magnitude);
        scalarMax = std::max(scalarMax, magnitude);
        ++buildResult.matchedNodeCount;
    }

    auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid->SetPoints(points);

    for (const TetraElement &element : meshData.tetraElements) {
        const auto node1 = nodeIdToVtkId.find(element.node1);
        const auto node2 = nodeIdToVtkId.find(element.node2);
        const auto node3 = nodeIdToVtkId.find(element.node3);
        const auto node4 = nodeIdToVtkId.find(element.node4);
        if (node1 == nodeIdToVtkId.end()
                || node2 == nodeIdToVtkId.end()
                || node3 == nodeIdToVtkId.end()
                || node4 == nodeIdToVtkId.end()) {
            buildResult.errors.append(QString("Tetrahedron %1 references a missing node.").arg(element.id));
            return buildResult;
        }

        vtkNew<vtkTetra> tetra;
        tetra->GetPointIds()->SetId(0, node1->second);
        tetra->GetPointIds()->SetId(1, node2->second);
        tetra->GetPointIds()->SetId(2, node3->second);
        tetra->GetPointIds()->SetId(3, node4->second);
        grid->InsertNextCell(tetra->GetCellType(), tetra->GetPointIds());
    }

    grid->GetPointData()->AddArray(displacementVector);
    grid->GetPointData()->AddArray(displacementMagnitudeArray);
    grid->GetPointData()->SetActiveVectors("Displacement");
    grid->GetPointData()->SetActiveScalars(buildResult.scalarName.toUtf8().constData());

    if (buildResult.matchedNodeCount < buildResult.meshNodeCount) {
        buildResult.warnings.append(QString("Only %1 of %2 mesh nodes have displacement values; missing nodes use 0.")
            .arg(buildResult.matchedNodeCount)
            .arg(buildResult.meshNodeCount));
    }

    buildResult.scalarMin = scalarMin == std::numeric_limits<double>::max() ? 0.0 : scalarMin;
    buildResult.scalarMax = scalarMax == std::numeric_limits<double>::lowest() ? 0.0 : scalarMax;
    buildResult.grid = grid;
    buildResult.success = true;
    return buildResult;
}
