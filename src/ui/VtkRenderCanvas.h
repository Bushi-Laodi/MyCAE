#pragma once

#include "geometry/BoxGeometry.h"
#include "picking/PickMode.h"
#include "picking/PickSelection.h"

#include <QString>
#include <QWidget>
#include <vtkSmartPointer.h>

#include <vector>

class QVTKOpenGLNativeWidget;
class TopoDS_Shape;
class vtkActor;
class vtkCallbackCommand;
class vtkGenericOpenGLRenderWindow;
class vtkObject;
class vtkPolyData;
class vtkRenderer;
class vtkScalarBarActor;
class vtkUnstructuredGrid;

class VtkRenderCanvas final : public QWidget
{
    Q_OBJECT

public:
    explicit VtkRenderCanvas(QWidget *parent = nullptr);

    void showEmpty();
    void showBoxGeometry(const BoxGeometry &box);
    void showOccShape(const TopoDS_Shape &shape, const QString &geometryName);
    void showMeshGrid(vtkSmartPointer<vtkUnstructuredGrid> grid);
    void showResultGrid(
        vtkSmartPointer<vtkUnstructuredGrid> grid,
        const QString &scalarName,
        double scalarMin,
        double scalarMax
    );
    void setPickMode(PickMode mode);
    void clearHighlight();
    void highlightFaceIndices(const std::vector<int> &faceIndices);

signals:
    void facePicked(const PickSelection &selection);

private:
    static void handleVtkLeftButtonRelease(vtkObject *caller, unsigned long eventId, void *clientData, void *callData);

    void resetCamera();
    void resetSceneState();
    void handlePickAtRenderWindowPosition(int x, int y);

    QVTKOpenGLNativeWidget *m_vtkWidget = nullptr;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkCallbackCommand> m_leftButtonReleaseCallback;
    vtkSmartPointer<vtkPolyData> m_currentPolyData;
    vtkSmartPointer<vtkActor> m_primaryActor;
    vtkSmartPointer<vtkActor> m_highlightActor;
    vtkSmartPointer<vtkScalarBarActor> m_scalarBarActor;
    QString m_activeGeometryName;
    PickMode m_pickMode = PickMode::None;
};
