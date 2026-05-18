#include "VtkRenderCanvas.h"

#include <algorithm>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

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

    m_renderer->SetBackground(0.125, 0.141, 0.165);
    showEmpty();
}

void VtkRenderCanvas::showEmpty()
{
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

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(cubeSource->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.84, 0.87, 0.90);
    actor->GetProperty()->SetEdgeColor(0.18, 0.24, 0.32);
    actor->GetProperty()->EdgeVisibilityOn();
    actor->GetProperty()->SetLineWidth(1.5);

    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(actor);
    m_renderer->ResetCamera();
    m_renderer->GetActiveCamera()->Azimuth(35.0);
    m_renderer->GetActiveCamera()->Elevation(25.0);
    m_renderer->ResetCameraClippingRange();
    m_renderWindow->Render();
}
