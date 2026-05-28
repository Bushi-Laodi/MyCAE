#include "render/VtkHighlightActorFactory.h"

#include "render/VtkFaceReferenceResolver.h"

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
    return createFaceHighlightActor(polyData, faceIndices, {});
}

vtkSmartPointer<vtkActor> VtkHighlightActorFactory::createFaceHighlightActor(
    vtkPolyData *polyData,
    const std::vector<int> &faceIndices,
    const std::vector<FaceReference> &faceReferences
)
{
    return createFaceHighlightActor(polyData, faceIndices, faceReferences, VtkHighlightStyle{});
}

vtkSmartPointer<vtkActor> VtkHighlightActorFactory::createFaceHighlightActor(
    vtkPolyData *polyData,
    const std::vector<int> &faceIndices,
    const std::vector<FaceReference> &faceReferences,
    const VtkHighlightStyle &style
)
{
    if (!polyData || (faceIndices.empty() && faceReferences.empty())) {
        return nullptr;
    }

    auto *faceIndexArray = vtkIntArray::SafeDownCast(polyData->GetCellData()->GetArray("MyCAE_FaceIndex"));
    if (!faceIndexArray) {
        return nullptr;
    }

    const std::vector<int> resolvedFaceIndices =
        VtkFaceReferenceResolver::resolveFaceIndices(polyData, faceIndices, faceReferences);
    if (resolvedFaceIndices.empty()) {
        return nullptr;
    }

    const std::set<int> selectedFaces(resolvedFaceIndices.begin(), resolvedFaceIndices.end());
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
    actor->GetProperty()->SetColor(style.red, style.green, style.blue);
    actor->GetProperty()->SetOpacity(style.opacity);
    actor->GetProperty()->SetEdgeColor(style.edgeRed, style.edgeGreen, style.edgeBlue);
    actor->GetProperty()->EdgeVisibilityOn();
    actor->GetProperty()->SetLineWidth(style.lineWidth);
    actor->GetProperty()->SetAmbient(0.35);
    actor->GetProperty()->SetDiffuse(0.75);
    return actor;
}
