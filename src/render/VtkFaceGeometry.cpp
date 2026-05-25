#include "render/VtkFaceGeometry.h"

#include <vtkCell.h>
#include <vtkMath.h>
#include <vtkPoints.h>

void VtkFaceGeometry::fillPickSelectionFromCell(vtkCell *cell, PickSelection &selection)
{
    if (!cell || !cell->GetPoints() || cell->GetNumberOfPoints() == 0) {
        return;
    }

    double center[3] = {0.0, 0.0, 0.0};
    const vtkIdType pointCount = cell->GetNumberOfPoints();
    for (vtkIdType pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
        double point[3] = {0.0, 0.0, 0.0};
        cell->GetPoints()->GetPoint(pointIndex, point);
        center[0] += point[0];
        center[1] += point[1];
        center[2] += point[2];
    }
    center[0] /= static_cast<double>(pointCount);
    center[1] /= static_cast<double>(pointCount);
    center[2] /= static_cast<double>(pointCount);

    double normal[3] = {0.0, 0.0, 0.0};
    double area = 0.0;
    if (pointCount >= 3) {
        double origin[3] = {0.0, 0.0, 0.0};
        cell->GetPoints()->GetPoint(0, origin);
        for (vtkIdType pointIndex = 1; pointIndex + 1 < pointCount; ++pointIndex) {
            double p1[3] = {0.0, 0.0, 0.0};
            double p2[3] = {0.0, 0.0, 0.0};
            cell->GetPoints()->GetPoint(pointIndex, p1);
            cell->GetPoints()->GetPoint(pointIndex + 1, p2);

            double v1[3] = {p1[0] - origin[0], p1[1] - origin[1], p1[2] - origin[2]};
            double v2[3] = {p2[0] - origin[0], p2[1] - origin[1], p2[2] - origin[2]};
            double cross[3] = {0.0, 0.0, 0.0};
            vtkMath::Cross(v1, v2, cross);
            area += 0.5 * vtkMath::Norm(cross);
            normal[0] += cross[0];
            normal[1] += cross[1];
            normal[2] += cross[2];
        }
        if (vtkMath::Norm(normal) > 0.0) {
            vtkMath::Normalize(normal);
        }
    }

    selection.centerX = center[0];
    selection.centerY = center[1];
    selection.centerZ = center[2];
    selection.normalX = normal[0];
    selection.normalY = normal[1];
    selection.normalZ = normal[2];
    selection.area = area;
}
