#include "MeshToVtkConverter.h"

#include <vtkNew.h>
#include <vtkIntArray.h>
#include <vtkCellData.h>
#include <vtkPoints.h>
#include <vtkQuadraticTetra.h>
#include <vtkSmartPointer.h>
#include <vtkTetra.h>
#include <vtkUnstructuredGrid.h>

#include <array>
#include <cstddef>
#include <unordered_map>

vtkSmartPointer<vtkUnstructuredGrid> MeshToVtkConverter::toUnstructuredGrid(
    const MeshData &meshData,
    QString *errorMessage
)
{
    if (meshData.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "MeshData is empty.";
        }
        return nullptr;
    }
    if (meshData.nodes.empty()) {
        if (errorMessage) {
            *errorMessage = "MeshData contains no nodes.";
        }
        return nullptr;
    }
    if (meshData.tetraElements.empty() && meshData.tetra10Elements.empty()) {
        if (errorMessage) {
            *errorMessage = "MeshData contains no tetrahedron elements.";
        }
        return nullptr;
    }

    vtkNew<vtkPoints> points;
    std::unordered_map<int, vtkIdType> nodeIdToVtkId;
    nodeIdToVtkId.reserve(meshData.nodes.size());

    for (const MeshNode &node : meshData.nodes) {
        const vtkIdType vtkPointId = points->InsertNextPoint(node.x, node.y, node.z);
        nodeIdToVtkId.insert({node.id, vtkPointId});
    }

    auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid->SetPoints(points);

    vtkNew<vtkIntArray> elementIds;
    elementIds->SetName("MyCAE_ElementId");

    for (const TetraElement &element : meshData.tetraElements) {
        const auto node1 = nodeIdToVtkId.find(element.node1);
        const auto node2 = nodeIdToVtkId.find(element.node2);
        const auto node3 = nodeIdToVtkId.find(element.node3);
        const auto node4 = nodeIdToVtkId.find(element.node4);
        if (node1 == nodeIdToVtkId.end()
                || node2 == nodeIdToVtkId.end()
                || node3 == nodeIdToVtkId.end()
                || node4 == nodeIdToVtkId.end()) {
            if (errorMessage) {
                *errorMessage = QString("Tetrahedron %1 references a missing node.").arg(element.id);
            }
            return nullptr;
        }

        vtkNew<vtkTetra> tetra;
        tetra->GetPointIds()->SetId(0, node1->second);
        tetra->GetPointIds()->SetId(1, node2->second);
        tetra->GetPointIds()->SetId(2, node3->second);
        tetra->GetPointIds()->SetId(3, node4->second);
        grid->InsertNextCell(tetra->GetCellType(), tetra->GetPointIds());
        elementIds->InsertNextValue(element.id);
    }

    for (const Tetra10Element &element : meshData.tetra10Elements) {
        const std::array<int, 10> nodeIds = {
            element.node1,
            element.node2,
            element.node3,
            element.node4,
            element.node5,
            element.node6,
            element.node7,
            element.node8,
            element.node9,
            element.node10
        };

        vtkNew<vtkQuadraticTetra> tetra;
        for (int index = 0; index < 10; ++index) {
            const auto node = nodeIdToVtkId.find(nodeIds[static_cast<size_t>(index)]);
            if (node == nodeIdToVtkId.end()) {
                if (errorMessage) {
                    *errorMessage = QString("Quadratic tetrahedron %1 references a missing node.").arg(element.id);
                }
                return nullptr;
            }
            tetra->GetPointIds()->SetId(index, node->second);
        }
        grid->InsertNextCell(tetra->GetCellType(), tetra->GetPointIds());
        elementIds->InsertNextValue(element.id);
    }

    if (grid->GetNumberOfPoints() == 0 || grid->GetNumberOfCells() == 0) {
        if (errorMessage) {
            *errorMessage = "VTK unstructured grid contains no points or cells.";
        }
        return nullptr;
    }

    grid->GetCellData()->AddArray(elementIds);
    return grid;
}
