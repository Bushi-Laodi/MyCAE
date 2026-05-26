#include "RenderView.h"

#include "geometry/BoxGeometry.h"
#include "VtkRenderCanvas.h"

#include <QLabel>
#include <QPixmap>
#include <QString>
#include <QVBoxLayout>

RenderView::RenderView(QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setObjectName("renderPlaceholder");
    setStyleSheet(
        "#renderPlaceholder {"
        "background: #20242a;"
        "border: 1px solid #3a3f46;"
        "}"
        "#renderPlaceholder QLabel {"
        "color: #d7dde5;"
        "font-size: 18px;"
        "}"
    );

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_subtitleLabel->setStyleSheet("font-size: 13px; color: #9ba7b4;");

    m_detailLabel = new QLabel(this);
    m_detailLabel->setAlignment(Qt::AlignCenter);
    m_detailLabel->setStyleSheet("font-size: 13px; color: #9ba7b4;");

    m_canvas = new VtkRenderCanvas(this);
    connect(m_canvas, &VtkRenderCanvas::facePicked, this, &RenderView::facePicked);
    connect(m_canvas, &VtkRenderCanvas::resultProbePicked, this, &RenderView::resultProbePicked);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_subtitleLabel);
    layout->addWidget(m_detailLabel);
    layout->addWidget(m_canvas, 1);

    // NOTE: Do NOT call showEmpty() here!
    // showEmpty() triggers VtkRenderCanvas::showEmpty() which calls
    // m_renderWindow->Render(). The OpenGL context may not be fully
    // ready during widget construction, causing a crash in MSVCP140.dll
    // (0xc0000005 - access violation).
    //
    // showEmpty() will be called lazily from the first paint event
    // or when the user first interacts with the render view.
}

void RenderView::showEmpty()
{
    m_titleLabel->setText("三维显示窗口");
    m_subtitleLabel->setText("请选择一个几何对象进行预览。");
    m_detailLabel->setText("VTK 渲染窗口已就绪。");
    m_canvas->showEmpty();
}

void RenderView::showBoxGeometry(const BoxGeometry &box)
{
    const QString sizeText = QString("%1 %4 x %2 %4 x %3 %4")
        .arg(box.length)
        .arg(box.width)
        .arg(box.height)
        .arg(box.unit);

    m_titleLabel->setText(box.name);
    m_subtitleLabel->setText(sizeText);
    m_detailLabel->setText("根据长方体参数生成的 VTK 立方体预览。");
    m_canvas->showBoxGeometry(box);
}

void RenderView::showOccShape(const TopoDS_Shape &shape, const QString &title, const QString &subtitle)
{
    m_titleLabel->setText(title);
    m_subtitleLabel->setText(subtitle);
    m_detailLabel->setText("Open CASCADE TopoDS_Shape converted to VTK PolyData.");
    m_canvas->showOccShape(shape, title);
}

void RenderView::showMeshGrid(vtkSmartPointer<vtkUnstructuredGrid> grid, const QString &title, const QString &subtitle)
{
    m_titleLabel->setText(title);
    m_subtitleLabel->setText(subtitle);
    m_detailLabel->setText("Gmsh tetrahedral mesh converted to VTK UnstructuredGrid.");
    m_canvas->showMeshGrid(grid);
}

void RenderView::showResultGrid(
    vtkSmartPointer<vtkUnstructuredGrid> grid,
    vtkSmartPointer<vtkUnstructuredGrid> overlayGrid,
    const QString &title,
    const QString &subtitle,
    const QString &scalarName,
    const QString &scalarUnit,
    bool useCellScalars,
    double scalarMin,
    double scalarMax,
    bool showMeshEdges,
    const ResultExtremeMarker &minimumMarker,
    const ResultExtremeMarker &maximumMarker,
    bool resetCamera
)
{
    m_titleLabel->setText(title);
    m_subtitleLabel->setText(subtitle);
    m_detailLabel->setText(QString("CalculiX scalar field: %1 [%2, %3] %4")
        .arg(scalarName)
        .arg(scalarMin, 0, 'g', 6)
        .arg(scalarMax, 0, 'g', 6)
        .arg(scalarUnit));
    m_canvas->showResultGrid(
        grid,
        overlayGrid,
        scalarName,
        scalarUnit,
        useCellScalars,
        scalarMin,
        scalarMax,
        showMeshEdges,
        minimumMarker,
        maximumMarker,
        resetCamera
    );
}

bool RenderView::saveScreenshot(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }
    return grab().save(filePath);
}

void RenderView::setPickMode(PickMode mode)
{
    m_canvas->setPickMode(mode);
}

void RenderView::clearHighlight()
{
    m_canvas->clearHighlight();
}

void RenderView::highlightFaceIndices(const std::vector<int> &faceIndices)
{
    m_canvas->highlightFaceIndices(faceIndices);
}

void RenderView::highlightResultPosition(double x, double y, double z)
{
    m_canvas->highlightResultPosition(x, y, z);
}

void RenderView::highlightResultExtrema(const ResultExtremeMarker &minimum, const ResultExtremeMarker &maximum)
{
    m_canvas->highlightResultExtrema(minimum, maximum);
}
