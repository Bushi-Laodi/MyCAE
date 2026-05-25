#include "render/VtkHighlightActorFactory.h"

#include <set>
#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkDataSetMapper.h>
#include <vtkExtractCells.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkMapper.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>

vtkSmartPointer<vtkActor> VtkHighlightActorFactory::createFaceHighlightActor(
    vtkPolyData *polyData,
    const std::vector<int> &faceIndices
)
{
    if (!polyData || faceIndices.empty()) {
        return nullptr;
    }

    auto *faceIndexArray = vtkIntArray::SafeDownCast(polyData->GetCellData()->GetArray("MyCAE_FaceIndex"));
    if (!faceIndexArray) {
        return nullptr;
    }

    const std::set<int> selectedFaces(faceIndices.begin(), faceIndices.end());
    vtkNew<vtkIdList> cellIds;
    for (vtkIdType cellId = 0; cellId < polyData->GetNumberOfCells(); ++cellId) {
        if (selectedFaces.find(faceIndexArray->GetValue(cellId)) != selectedFaces.end()) {
            cellIds->InsertNextId(cellId);
        }
    }
    if (cellIds->GetNumberOfIds() == 0) {
        return nullptr;
    }

    vtkNew<vtkExtractCells> extractCells;
    extractCells->SetInputData(polyData);
    extractCells->SetCellList(cellIds);
    extractCells->Update();

    vtkNew<vtkDataSetMapper> mapper;
    mapper->SetInputConnection(extractCells->GetOutputPort());
    mapper->ScalarVisibilityOff();
    vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
    vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(-1.0, -1.0);

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1.0, 0.76, 0.08);
    actor->GetProperty()->SetOpacity(0.92);
    actor->GetProperty()->SetEdgeColor(1.0, 0.95, 0.55);
    actor->GetProperty()->EdgeVisibilityOn();
    actor->GetProperty()->SetLineWidth(3.0);
    actor->GetProperty()->SetAmbient(0.35);
    actor->GetProperty()->SetDiffuse(0.75);
    return actor;
}
