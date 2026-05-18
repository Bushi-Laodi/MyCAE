#pragma once

#include "geometry/BoxGeometry.h"

#include <QWidget>
#include <vtkSmartPointer.h>

class QVTKOpenGLNativeWidget;
class TopoDS_Shape;
class vtkGenericOpenGLRenderWindow;
class vtkRenderer;
class vtkUnstructuredGrid;

class VtkRenderCanvas final : public QWidget
{
public:
    explicit VtkRenderCanvas(QWidget *parent = nullptr);

    void showEmpty();
    void showBoxGeometry(const BoxGeometry &box);
    void showOccShape(const TopoDS_Shape &shape);
    void showMeshGrid(vtkSmartPointer<vtkUnstructuredGrid> grid);

private:
    void resetCamera();

    QVTKOpenGLNativeWidget *m_vtkWidget = nullptr;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
};
