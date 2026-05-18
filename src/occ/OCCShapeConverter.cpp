#include "OCCShapeConverter.h"

#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangle.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

#include <vtkCellArray.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <stdexcept>
#include <vector>

vtkSmartPointer<vtkPolyData> OCCShapeConverter::toPolyData(const TopoDS_Shape &shape) const
{
    if (shape.IsNull()) {
        throw std::invalid_argument("Cannot convert a null TopoDS_Shape.");
    }

    constexpr double linearDeflection = 0.5;
    constexpr bool isRelative = false;
    constexpr double angularDeflection = 0.5;
    BRepMesh_IncrementalMesh mesh(shape, linearDeflection, isRelative, angularDeflection);
    mesh.Perform();
    if (!mesh.IsDone()) {
        throw std::runtime_error("Open CASCADE meshing failed.");
    }

    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> triangles;

    for (TopExp_Explorer faceExplorer(shape, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
        const TopoDS_Face face = TopoDS::Face(faceExplorer.Current());

        TopLoc_Location location;
        const Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
        if (triangulation.IsNull()) {
            continue;
        }

        const gp_Trsf transform = location.Transformation();
        const vtkIdType pointOffset = points->GetNumberOfPoints();
        for (int nodeIndex = 1; nodeIndex <= triangulation->NbNodes(); ++nodeIndex) {
            gp_Pnt point = triangulation->Node(nodeIndex);
            point.Transform(transform);
            points->InsertNextPoint(point.X(), point.Y(), point.Z());
        }

        for (int triangleIndex = 1; triangleIndex <= triangulation->NbTriangles(); ++triangleIndex) {
            int n1 = 0;
            int n2 = 0;
            int n3 = 0;
            triangulation->Triangle(triangleIndex).Get(n1, n2, n3);

            if (face.Orientation() == TopAbs_REVERSED) {
                std::swap(n2, n3);
            }

            const vtkIdType ids[3] = {
                pointOffset + static_cast<vtkIdType>(n1 - 1),
                pointOffset + static_cast<vtkIdType>(n2 - 1),
                pointOffset + static_cast<vtkIdType>(n3 - 1)
            };
            triangles->InsertNextCell(3, ids);
        }
    }

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(triangles);
    polyData->BuildCells();
    polyData->BuildLinks();

    if (polyData->GetNumberOfPoints() == 0 || polyData->GetNumberOfPolys() == 0) {
        throw std::runtime_error("Open CASCADE shape produced no VTK triangles.");
    }

    return polyData;
}
