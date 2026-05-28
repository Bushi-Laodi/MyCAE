#include "VtkRenderCanvas.h"

#include "geometry/FaceGroup.h"
#include "occ/OCCShapeConverter.h"
#include "render/VtkHighlightActorFactory.h"
#include "render/VtkPickAdapter.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSizePolicy>
#include <QTimer>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellPicker.h>
#include <vtkCommand.h>
#include <vtkCubeSource.h>
#include <vtkDataArray.h>
#include <vtkDataSetMapper.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkObject.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkSphereSource.h>
#include <vtkUnstructuredGrid.h>

namespace
{
int intTupleValue(vtkDataArray *array, vtkIdType index)
{
    return array ? static_cast<int>(array->GetTuple1(index)) : 0;
}

double doubleTupleValue(vtkDataArray *array, vtkIdType index)
{
    return array ? array->GetTuple1(index) : 0.0;
}

double squaredDistance(const double *a, const double *b)
{
    const double dx = a[0] - b[0];
    const double dy = a[1] - b[1];
    const double dz = a[2] - b[2];
    return dx * dx + dy * dy + dz * dz;
}
}

VtkRenderCanvas::VtkRenderCanvas(QWidget *parent)
    : QWidget(parent)
    , m_renderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New())
    , m_renderer(vtkSmartPointer<vtkRenderer>::New())
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    m_vtkWidget->setMinimumHeight(260);
    m_vtkWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_vtkWidget);

    m_renderWindow->AddRenderer(m_renderer);
    m_vtkWidget->setRenderWindow(m_renderWindow);
    m_leftButtonReleaseCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    m_leftButtonReleaseCallback->SetClientData(this);
    m_leftButtonReleaseCallback->SetCallback(&VtkRenderCanvas::handleVtkLeftButtonRelease);
    if (m_renderWindow->GetInteractor()) {
        m_renderWindow->GetInteractor()->AddObserver(
            vtkCommand::LeftButtonReleaseEvent,
            m_leftButtonReleaseCallback
        );
    }

    m_renderer->SetBackground(0.125, 0.141, 0.165);

    // NOTE: Do NOT call showEmpty() / Render() here!
    // The OpenGL context is not fully ready during widget construction.
    // Calling Render() at this point can cause a crash in MSVCP140.dll
    // (0xc0000005 - access violation) because the underlying OpenGL
    // context may not be fully initialized yet.
    //
    // showEmpty() will be called lazily from RenderView::showEmpty()
    // or from the first paint event.
}

VtkRenderCanvas::~VtkRenderCanvas()
{
    if (m_renderWindow && m_renderWindow->GetInteractor() && m_leftButtonReleaseCallback) {
        m_renderWindow->GetInteractor()->RemoveObserver(m_leftButtonReleaseCallback);
    }
    if (m_vtkWidget) {
        m_vtkWidget->setRenderWindow(static_cast<vtkGenericOpenGLRenderWindow *>(nullptr));
    }
    if (m_renderWindow) {
        m_renderWindow->Finalize();
    }
    m_scalarBarActor = nullptr;
    m_highlightActors.clear();
    m_primaryActor = nullptr;
    m_currentResultGrid = nullptr;
    m_currentPolyData = nullptr;
    m_renderer = nullptr;
    m_renderWindow = nullptr;
}

void VtkRenderCanvas::showEmpty()
{
    resetSceneState();
    m_renderer->RemoveAllViewProps();
    requestRender();
}

void VtkRenderCanvas::showBoxGeometry(const BoxGeometry &box)
{
    const double length = std::max(0.001, box.length);
    const double width = std::max(0.001, box.width);
    const double height = std::max(0.001, box.height);

    vtkNew<vtkCubeSource> cubeSource;
    cubeSource->SetXLength(length);
    cubeSource->SetYLength(width);
    cubeSource->SetZLength(height);
    cubeSource->SetCenter(0.0, 0.0, 0.0);
    cubeSource->Update();

    vtkSmartPointer<vtkPolyData> polyData = cubeSource->GetOutput();
    vtkNew<vtkIntArray> faceIndices;
    faceIndices->SetName("MyCAE_FaceIndex");
    for (vtkIdType cellId = 0; cellId < polyData->GetNumberOfCells(); ++cellId) {
        faceIndices->InsertNextValue(static_cast<int>(cellId + 1));
    }
    polyData->GetCellData()->AddArray(faceIndices);
    polyData->GetCellData()->SetActiveScalars("MyCAE_FaceIndex");

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(polyData);

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.84, 0.87, 0.90);
    actor->GetProperty()->SetEdgeColor(0.18, 0.24, 0.32);
    actor->GetProperty()->EdgeVisibilityOn();
    actor->GetProperty()->SetLineWidth(1.5);

    resetSceneState();
    m_currentPolyData = polyData;
    m_activeGeometryName = box.name;
    m_primaryActor = actor;
    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(m_primaryActor);
    resetCamera();
    requestRender();
}

void VtkRenderCanvas::showOccShape(const TopoDS_Shape &shape, const QString &geometryName)
{
    OCCShapeConverter converter;
    vtkSmartPointer<vtkPolyData> polyData = converter.toPolyData(shape);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(polyData);

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.62, 0.78, 0.95);
    actor->GetProperty()->SetEdgeColor(0.12, 0.18, 0.24);
    actor->GetProperty()->EdgeVisibilityOn();
    actor->GetProperty()->SetLineWidth(1.2);

    resetSceneState();
    m_currentPolyData = polyData;
    m_activeGeometryName = geometryName;
    m_primaryActor = actor;
    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(m_primaryActor);
    resetCamera();
    requestRender();
}

void VtkRenderCanvas::showMeshGrid(vtkSmartPointer<vtkUnstructuredGrid> grid)
{
    vtkNew<vtkDataSetMapper> mapper;
    mapper->SetInputData(grid);

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.42, 0.72, 0.54);
    actor->GetProperty()->SetEdgeColor(0.08, 0.16, 0.11);
    actor->GetProperty()->EdgeVisibilityOn();
    actor->GetProperty()->SetLineWidth(0.8);
    actor->GetProperty()->SetOpacity(0.78);

    resetSceneState();
    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(actor);
    resetCamera();
    requestRender();
}

void VtkRenderCanvas::showResultGrid(
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
    bool resetCameraView
)
{
    if (scalarMin == scalarMax) {
        const double delta = scalarMin == 0.0 ? 1.0 : std::abs(scalarMin) * 0.05;
        scalarMin -= delta;
        scalarMax += delta;
    }

    vtkNew<vtkLookupTable> lookupTable;
    lookupTable->SetHueRange(0.667, 0.0);
    lookupTable->SetSaturationRange(0.82, 0.92);
    lookupTable->SetValueRange(0.90, 1.0);
    lookupTable->SetNumberOfTableValues(256);
    lookupTable->SetRange(scalarMin, scalarMax);
    lookupTable->Build();

    vtkNew<vtkDataSetMapper> mapper;
    mapper->SetInputData(grid);
    mapper->SetLookupTable(lookupTable);
    mapper->SetScalarRange(scalarMin, scalarMax);
    mapper->ScalarVisibilityOn();
    if (useCellScalars) {
        mapper->SetScalarModeToUseCellFieldData();
    } else {
        mapper->SetScalarModeToUsePointFieldData();
    }
    mapper->SelectColorArray(scalarName.toUtf8().constData());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetEdgeColor(0.08, 0.10, 0.12);
    actor->GetProperty()->SetEdgeVisibility(showMeshEdges);
    actor->GetProperty()->SetLineWidth(0.45);
    actor->GetProperty()->SetOpacity(0.96);

    vtkNew<vtkScalarBarActor> scalarBar;
    scalarBar->SetLookupTable(lookupTable);
    const QString title = scalarUnit.isEmpty() ? scalarName : scalarName + " (" + scalarUnit + ")";
    scalarBar->SetTitle(title.toUtf8().constData());
    scalarBar->SetLabelFormat("%.3e");
    scalarBar->SetNumberOfLabels(5);
    scalarBar->SetMaximumWidthInPixels(90);
    scalarBar->SetMaximumHeightInPixels(420);

    resetSceneState();
    m_currentResultGrid = grid;
    m_primaryActor = actor;
    m_scalarBarActor = scalarBar;
    m_renderer->RemoveAllViewProps();
    if (overlayGrid) {
        vtkNew<vtkDataSetMapper> overlayMapper;
        overlayMapper->SetInputData(overlayGrid);

        vtkNew<vtkActor> overlayActor;
        overlayActor->SetMapper(overlayMapper);
        overlayActor->GetProperty()->SetColor(0.88, 0.88, 0.88);
        overlayActor->GetProperty()->SetOpacity(0.18);
        overlayActor->GetProperty()->SetEdgeColor(0.15, 0.15, 0.15);
        overlayActor->GetProperty()->EdgeVisibilityOn();
        overlayActor->GetProperty()->SetLineWidth(0.35);
        m_renderer->AddActor(overlayActor);
    }
    m_renderer->AddActor(m_primaryActor);
    if (minimumMarker.valid) {
        addResultMarker(minimumMarker.x, minimumMarker.y, minimumMarker.z, 0.12, 0.46, 0.98, 0.022);
    }
    if (maximumMarker.valid) {
        addResultMarker(maximumMarker.x, maximumMarker.y, maximumMarker.z, 1.0, 0.32, 0.18, 0.026);
    }
    m_renderer->AddActor2D(m_scalarBarActor);
    if (resetCameraView) {
        resetCamera();
    }
    requestRender();
}

void VtkRenderCanvas::setPickMode(PickMode mode)
{
    m_pickMode = mode;
}

void VtkRenderCanvas::clearHighlight()
{
    if (!m_highlightActors.empty()) {
        for (vtkActor *actor : m_highlightActors) {
            if (actor) {
                m_renderer->RemoveActor(actor);
            }
        }
        m_highlightActors.clear();
        requestRender();
    }
}

void VtkRenderCanvas::highlightFaceGroup(const FaceGroup &faceGroup, const VtkHighlightStyle &style)
{
    clearHighlight();
    vtkSmartPointer<vtkActor> actor = VtkHighlightActorFactory::createFaceHighlightActor(
        m_currentPolyData,
        faceGroup.faceIndices,
        faceGroup.faceReferences,
        style
    );
    if (actor) {
        m_highlightActors.push_back(actor);
        m_renderer->AddActor(actor);
    }
    requestRender();
}

void VtkRenderCanvas::highlightFaceIndices(const std::vector<int> &faceIndices)
{
    clearHighlight();
    vtkSmartPointer<vtkActor> actor = VtkHighlightActorFactory::createFaceHighlightActor(m_currentPolyData, faceIndices);
    if (actor) {
        m_highlightActors.push_back(actor);
        m_renderer->AddActor(actor);
    }
    requestRender();
}

void VtkRenderCanvas::highlightResultPosition(double x, double y, double z)
{
    clearHighlight();
    addResultMarker(x, y, z, 1.0, 0.86, 0.18, 0.025);
    requestRender();
}

void VtkRenderCanvas::highlightResultExtrema(const ResultExtremeMarker &minimum, const ResultExtremeMarker &maximum)
{
    clearHighlight();
    if (minimum.valid) {
        addResultMarker(minimum.x, minimum.y, minimum.z, 0.12, 0.46, 0.98, 0.022);
    }
    if (maximum.valid) {
        addResultMarker(maximum.x, maximum.y, maximum.z, 1.0, 0.32, 0.18, 0.026);
    }
    requestRender();
}

QString VtkRenderCanvas::activeGeometryName() const
{
    return m_activeGeometryName;
}

void VtkRenderCanvas::addResultMarker(
    double x,
    double y,
    double z,
    double red,
    double green,
    double blue,
    double radiusScale
)
{
    double bounds[6] = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
    if (m_primaryActor) {
        m_primaryActor->GetBounds(bounds);
    }
    const double diagonal = std::sqrt(
        (bounds[1] - bounds[0]) * (bounds[1] - bounds[0])
        + (bounds[3] - bounds[2]) * (bounds[3] - bounds[2])
        + (bounds[5] - bounds[4]) * (bounds[5] - bounds[4])
    );
    const double radius = std::max(0.001, diagonal * radiusScale);

    vtkNew<vtkSphereSource> sphere;
    sphere->SetCenter(x, y, z);
    sphere->SetRadius(radius);
    sphere->SetThetaResolution(24);
    sphere->SetPhiResolution(16);
    sphere->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(sphere->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(red, green, blue);
    actor->GetProperty()->SetAmbient(0.35);
    actor->GetProperty()->SetDiffuse(0.85);
    actor->GetProperty()->SetSpecular(0.25);

    m_highlightActors.push_back(actor);
    m_renderer->AddActor(actor);
}

void VtkRenderCanvas::handleVtkLeftButtonRelease(
    vtkObject *caller,
    unsigned long eventId,
    void *clientData,
    void *callData
)
{
    Q_UNUSED(eventId);
    Q_UNUSED(callData);

    auto *canvas = static_cast<VtkRenderCanvas *>(clientData);
    auto *interactor = vtkRenderWindowInteractor::SafeDownCast(caller);
    if (!canvas || !interactor) {
        return;
    }

    const int *position = interactor->GetEventPosition();
    canvas->handlePickAtRenderWindowPosition(position[0], position[1]);
}

void VtkRenderCanvas::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    requestRender();
}

void VtkRenderCanvas::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    requestRender();
}

void VtkRenderCanvas::requestRender()
{
    if (m_renderQueued) {
        return;
    }

    renderIfReady();
    m_renderQueued = true;
    QTimer::singleShot(0, this, [this]() {
        m_renderQueued = false;
        if (!renderIfReady() && m_vtkWidget) {
            m_vtkWidget->update();
        }
    });
}

bool VtkRenderCanvas::renderIfReady()
{
    if (!m_renderWindow || !m_renderer || !m_vtkWidget || !m_vtkWidget->isVisible() || m_vtkWidget->size().isEmpty()) {
        return false;
    }
    m_renderer->Modified();
    m_renderWindow->Modified();
    m_renderWindow->Render();
    m_vtkWidget->update();
    m_vtkWidget->repaint();
    return true;
}

void VtkRenderCanvas::resetCamera()
{
    m_renderer->ResetCamera();
    m_renderer->GetActiveCamera()->Azimuth(35.0);
    m_renderer->GetActiveCamera()->Elevation(25.0);
    m_renderer->ResetCameraClippingRange();
    requestRender();
}

void VtkRenderCanvas::resetSceneState()
{
    m_currentPolyData = nullptr;
    m_currentResultGrid = nullptr;
    m_primaryActor = nullptr;
    m_highlightActors.clear();
    m_scalarBarActor = nullptr;
    m_activeGeometryName.clear();
}

void VtkRenderCanvas::handlePickAtRenderWindowPosition(int x, int y)
{
    if (m_pickMode != PickMode::Face || !m_currentPolyData || !m_primaryActor || m_activeGeometryName.isEmpty()) {
        ResultProbe probe;
        if (pickResultAtRenderWindowPosition(x, y, probe)) {
            highlightResultPosition(probe.x, probe.y, probe.z);
            emit resultProbePicked(probe);
        }
        return;
    }

    PickSelection selection;
    if (VtkPickAdapter::pickFace(m_renderer, m_primaryActor, m_currentPolyData, m_activeGeometryName, x, y, selection)) {
        emit facePicked(selection);
    }
}

bool VtkRenderCanvas::pickResultAtRenderWindowPosition(int x, int y, ResultProbe &probe)
{
    if (!m_currentResultGrid || !m_primaryActor) {
        return false;
    }

    vtkNew<vtkCellPicker> picker;
    picker->SetTolerance(0.0025);
    if (!picker->Pick(x, y, 0.0, m_renderer)) {
        return false;
    }
    if (picker->GetActor() != m_primaryActor) {
        return false;
    }

    const vtkIdType cellId = picker->GetCellId();
    if (cellId < 0 || cellId >= m_currentResultGrid->GetNumberOfCells()) {
        return false;
    }

    double pickedPosition[3] = {0.0, 0.0, 0.0};
    picker->GetPickPosition(pickedPosition);

    vtkCell *cell = m_currentResultGrid->GetCell(cellId);
    if (!cell || !cell->GetPointIds() || cell->GetPointIds()->GetNumberOfIds() <= 0) {
        return false;
    }

    vtkIdType nearestPointId = cell->GetPointId(0);
    double nearestPoint[3] = {0.0, 0.0, 0.0};
    m_currentResultGrid->GetPoint(nearestPointId, nearestPoint);
    double nearestDistance = squaredDistance(nearestPoint, pickedPosition);
    for (vtkIdType i = 1; i < cell->GetPointIds()->GetNumberOfIds(); ++i) {
        const vtkIdType pointId = cell->GetPointId(i);
        double point[3] = {0.0, 0.0, 0.0};
        m_currentResultGrid->GetPoint(pointId, point);
        const double distance = squaredDistance(point, pickedPosition);
        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearestPointId = pointId;
            nearestPoint[0] = point[0];
            nearestPoint[1] = point[1];
            nearestPoint[2] = point[2];
        }
    }

    vtkPointData *pointData = m_currentResultGrid->GetPointData();
    vtkCellData *cellData = m_currentResultGrid->GetCellData();

    probe.valid = true;
    probe.source = "Render pick";
    probe.elementId = intTupleValue(cellData ? cellData->GetArray("MyCAE_ElementId") : nullptr, cellId);
    probe.nodeId = intTupleValue(pointData ? pointData->GetArray("MyCAE_NodeId") : nullptr, nearestPointId);
    probe.x = nearestPoint[0];
    probe.y = nearestPoint[1];
    probe.z = nearestPoint[2];
    probe.ux = doubleTupleValue(pointData ? pointData->GetArray("Ux") : nullptr, nearestPointId);
    probe.uy = doubleTupleValue(pointData ? pointData->GetArray("Uy") : nullptr, nearestPointId);
    probe.uz = doubleTupleValue(pointData ? pointData->GetArray("Uz") : nullptr, nearestPointId);
    probe.displacementMagnitude =
        doubleTupleValue(pointData ? pointData->GetArray("Displacement Magnitude") : nullptr, nearestPointId);
    probe.vonMisesStress =
        doubleTupleValue(cellData ? cellData->GetArray("Von Mises Stress") : nullptr, cellId);
    return true;
}
