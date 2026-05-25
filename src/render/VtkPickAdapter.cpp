#include "render/VtkPickAdapter.h"

#include "render/VtkFaceGeometry.h"

#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkCellPicker.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>

bool VtkPickAdapter::pickFace(
    vtkRenderer *renderer,
    vtkActor *actor,
    vtkPolyData *polyData,
    const QString &geometryName,
    int x,
    int y,
    PickSelection &selection
)
{
    if (!renderer || !actor || !polyData || geometryName.isEmpty()) {
        return false;
    }

    vtkNew<vtkCellPicker> picker;
    picker->SetTolerance(0.0008);
    picker->PickFromListOn();
    picker->AddPickList(actor);
    if (!picker->Pick(x, y, 0.0, renderer)) {
        return false;
    }

    const vtkIdType cellId = picker->GetCellId();
    if (cellId < 0 || cellId >= polyData->GetNumberOfCells()) {
        return false;
    }

    auto *faceIndexArray = vtkIntArray::SafeDownCast(polyData->GetCellData()->GetArray("MyCAE_FaceIndex"));
    if (!faceIndexArray) {
        return false;
    }

    double pickedPosition[3] = {0.0, 0.0, 0.0};
    picker->GetPickPosition(pickedPosition);

    selection = {};
    selection.mode = PickMode::Face;
    selection.geometryName = geometryName;
    selection.topologyIndex = faceIndexArray->GetValue(cellId);
    selection.cellId = static_cast<long long>(cellId);
    selection.worldX = pickedPosition[0];
    selection.worldY = pickedPosition[1];
    selection.worldZ = pickedPosition[2];
    VtkFaceGeometry::fillPickSelectionFromCell(polyData->GetCell(cellId), selection);
    return selection.isValid();
}
