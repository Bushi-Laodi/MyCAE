#include "VtkRenderCanvas.h"

#include "occ/OCCShapeConverter.h"
#include "render/VtkHighlightActorFactory.h"
#include "render/VtkPickAdapter.h"

#include <algorithm>
#include <vector>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkCubeSource.h>
#include <vtkDataSetMapper.h>
#include <vtkGenericOpenGLRenderWindow.h>
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
#include <vtkUnstructuredGrid.h>

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

void VtkRenderCanvas::showEmpty()
{
    resetSceneState();
    m_renderer->RemoveAllViewProps();
    m_renderWindow->Render();
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
    m_renderWindow->Render();
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
    m_renderWindow->Render();
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
    m_renderWindow->Render();
}

void VtkRenderCanvas::showResultGrid(
    vtkSmartPointer<vtkUnstructuredGrid> grid,
    const QString &scalarName,
    double scalarMin,
    double scalarMax
)
{
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
    mapper->SetScalarModeToUsePointFieldData();
    mapper->SelectColorArray(scalarName.toUtf8().constData());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetEdgeColor(0.08, 0.10, 0.12);
    actor->GetProperty()->EdgeVisibilityOn();
    actor->GetProperty()->SetLineWidth(0.45);
    actor->GetProperty()->SetOpacity(0.96);

    vtkNew<vtkScalarBarActor> scalarBar;
    scalarBar->SetLookupTable(lookupTable);
    scalarBar->SetTitle(scalarName.toUtf8().constData());
    scalarBar->SetNumberOfLabels(5);
    scalarBar->SetMaximumWidthInPixels(90);
    scalarBar->SetMaximumHeightInPixels(420);

    resetSceneState();
    m_primaryActor = actor;
    m_scalarBarActor = scalarBar;
    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(m_primaryActor);
    m_renderer->AddActor2D(m_scalarBarActor);
    resetCamera();
    m_renderWindow->Render();
}

void VtkRenderCanvas::setPickMode(PickMode mode)
{
    m_pickMode = mode;
}

void VtkRenderCanvas::clearHighlight()
{
    if (m_highlightActor) {
        m_renderer->RemoveActor(m_highlightActor);
        m_highlightActor = nullptr;
        m_renderWindow->Render();
    }
}

void VtkRenderCanvas::highlightFaceIndices(const std::vector<int> &faceIndices)
{
    clearHighlight();
    m_highlightActor = VtkHighlightActorFactory::createFaceHighlightActor(m_currentPolyData, faceIndices);
    if (m_highlightActor) {
        m_renderer->AddActor(m_highlightActor);
    }
    m_renderWindow->Render();
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

void VtkRenderCanvas::resetCamera()
{
    m_renderer->ResetCamera();
    m_renderer->GetActiveCamera()->Azimuth(35.0);
    m_renderer->GetActiveCamera()->Elevation(25.0);
    m_renderer->ResetCameraClippingRange();
    m_renderWindow->Render();
}

void VtkRenderCanvas::resetSceneState()
{
    m_currentPolyData = nullptr;
    m_primaryActor = nullptr;
    m_highlightActor = nullptr;
    m_scalarBarActor = nullptr;
    m_activeGeometryName.clear();
}

void VtkRenderCanvas::handlePickAtRenderWindowPosition(int x, int y)
{
    if (m_pickMode != PickMode::Face || !m_currentPolyData || !m_primaryActor || m_activeGeometryName.isEmpty()) {
        return;
    }

    PickSelection selection;
    if (VtkPickAdapter::pickFace(m_renderer, m_primaryActor, m_currentPolyData, m_activeGeometryName, x, y, selection)) {
        emit facePicked(selection);
    }
}
