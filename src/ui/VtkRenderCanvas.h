#pragma once

#include "geometry/BoxGeometry.h"
#include "picking/PickMode.h"
#include "picking/PickSelection.h"
#include "render/RenderDisplaySettings.h"
#include "render/VtkHighlightActorFactory.h"
#include "result/ResultExtrema.h"
#include "result/ResultProbe.h"
#include "ui/RenderGeometryItem.h"

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
class vtkOrientationMarkerWidget;
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
    void showGeometryScene(
        const std::vector<RenderGeometryItem> &items,
        const QString &selectedGeometryName,
        bool resetCamera = true
    );
    void showMeshGrid(vtkSmartPointer<vtkUnstructuredGrid> grid);
    void setMeshTransparent(bool transparent);
    bool meshTransparent() const;
    void setRenderDisplaySettings(const RenderDisplaySettings &settings);
    RenderDisplaySettings renderDisplaySettings() const;
    void resetRenderDisplaySettings();
    void setPrimaryOpacity(double opacity);
    double primaryOpacity() const;
    void setResultOpacity(double opacity);
    double resultOpacity() const;
    void setUndeformedOverlayOpacity(double opacity);
    double undeformedOverlayOpacity() const;
    void setHighlightOpacity(double opacity);
    double highlightOpacity() const;
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
    void setGeometryEdgesVisible(bool visible);
    bool geometryEdgesVisible() const;
    void setOrientationMarkerVisible(bool visible);
    bool orientationMarkerVisible() const;
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
    void initializeOrientationMarker();
    void applyPrimaryGeometryEdgeVisibility();
    void applyGeometrySceneEdgeVisibility();
    void applyRenderDisplaySettings();
    double clampedOpacity(double value) const;
    bool effectiveGeometryEdgesVisible() const;
    void handlePickAtRenderWindowPosition(int x, int y);
    bool pickResultAtRenderWindowPosition(int x, int y, ResultProbe &probe);
    void addResultMarker(double x, double y, double z, double red, double green, double blue, double radiusScale);

    QVTKOpenGLNativeWidget *m_vtkWidget = nullptr;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkCallbackCommand> m_leftButtonReleaseCallback;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_orientationMarker;
    vtkSmartPointer<vtkPolyData> m_currentPolyData;
    vtkSmartPointer<vtkUnstructuredGrid> m_currentResultGrid;
    vtkSmartPointer<vtkActor> m_primaryActor;
    vtkSmartPointer<vtkActor> m_resultActor;
    vtkSmartPointer<vtkActor> m_undeformedOverlayActor;
    std::vector<vtkSmartPointer<vtkActor>> m_geometrySceneActors;
    std::vector<vtkSmartPointer<vtkActor>> m_highlightActors;
    vtkSmartPointer<vtkScalarBarActor> m_scalarBarActor;
    QString m_activeGeometryName;
    RenderDisplaySettings m_renderSettings;
    PickMode m_pickMode = PickMode::None;
    bool m_primaryActorIsMesh = false;
    bool m_primaryActorUsesGeometryEdges = false;
    bool m_renderQueued = false;
};
