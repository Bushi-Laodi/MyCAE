#pragma once

#include "geometry/BoxGeometry.h"
#include "picking/PickMode.h"
#include "picking/PickSelection.h"
#include "render/VtkHighlightActorFactory.h"
#include "result/ResultExtrema.h"
#include "result/ResultProbe.h"

#include <QString>
#include <QWidget>
#include <vtkSmartPointer.h>

#include <vector>

class QVTKOpenGLNativeWidget;
class QResizeEvent;
class QShowEvent;
class TopoDS_Shape;
class vtkActor;
class vtkCallbackCommand;
class vtkGenericOpenGLRenderWindow;
class vtkObject;
class vtkPolyData;
class vtkRenderer;
class vtkScalarBarActor;
class vtkUnstructuredGrid;
struct FaceGroup;

class VtkRenderCanvas final : public QWidget
{
    Q_OBJECT

public:
    explicit VtkRenderCanvas(QWidget *parent = nullptr);
    ~VtkRenderCanvas() override;

    void showEmpty();
    void showBoxGeometry(const BoxGeometry &box);
    void showOccShape(const TopoDS_Shape &shape, const QString &geometryName);
    void showMeshGrid(vtkSmartPointer<vtkUnstructuredGrid> grid);
    void showResultGrid(
        vtkSmartPointer<vtkUnstructuredGrid> grid,
        vtkSmartPointer<vtkUnstructuredGrid> overlayGrid,
        const QString &scalarName,
        const QString &scalarUnit,
        bool useCellScalars,
        double scalarMin,
        double scalarMax,
        bool showMeshEdges,
        const ResultExtremeMarker &minimumMarker,
        const ResultExtremeMarker &maximumMarker,
        bool resetCamera = true
    );
    void setPickMode(PickMode mode);
    void clearHighlight();
    void highlightFaceGroup(const FaceGroup &faceGroup, const VtkHighlightStyle &style = VtkHighlightStyle{});
    void highlightFaceIndices(const std::vector<int> &faceIndices);
    void highlightResultPosition(double x, double y, double z);
    void highlightResultExtrema(const ResultExtremeMarker &minimum, const ResultExtremeMarker &maximum);
    QString activeGeometryName() const;

signals:
    void facePicked(const PickSelection &selection);
    void resultProbePicked(const ResultProbe &probe);

private:
    static void handleVtkLeftButtonRelease(vtkObject *caller, unsigned long eventId, void *clientData, void *callData);

    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void requestRender();
    bool renderIfReady();
    void resetCamera();
    void resetSceneState();
    void handlePickAtRenderWindowPosition(int x, int y);
    bool pickResultAtRenderWindowPosition(int x, int y, ResultProbe &probe);
    void addResultMarker(double x, double y, double z, double red, double green, double blue, double radiusScale);

    QVTKOpenGLNativeWidget *m_vtkWidget = nullptr;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkCallbackCommand> m_leftButtonReleaseCallback;
    vtkSmartPointer<vtkPolyData> m_currentPolyData;
    vtkSmartPointer<vtkUnstructuredGrid> m_currentResultGrid;
    vtkSmartPointer<vtkActor> m_primaryActor;
    std::vector<vtkSmartPointer<vtkActor>> m_highlightActors;
    vtkSmartPointer<vtkScalarBarActor> m_scalarBarActor;
    QString m_activeGeometryName;
    PickMode m_pickMode = PickMode::None;
    bool m_renderQueued = false;
};
