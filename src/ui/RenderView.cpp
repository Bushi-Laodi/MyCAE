#include "RenderView.h"

#include "geometry/BoxGeometry.h"
#include "geometry/FaceGroup.h"
#include "VtkRenderCanvas.h"

#include <QCoreApplication>
#include <QLabel>
#include <QPixmap>
#include <QString>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}
}

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

    if (!qApp->property("mycae.skipVtkCanvas").toBool()) {
        m_canvas = new VtkRenderCanvas(this);
        connect(m_canvas, &VtkRenderCanvas::facePicked, this, &RenderView::facePicked);
        connect(m_canvas, &VtkRenderCanvas::resultProbePicked, this, &RenderView::resultProbePicked);
    }

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_subtitleLabel);
    layout->addWidget(m_detailLabel);
    if (m_canvas) {
        layout->addWidget(m_canvas, 1);
    } else {
        layout->addStretch(1);
    }

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
    m_titleLabel->setText(zh(u8"三维显示窗口"));
    m_subtitleLabel->setText(zh(u8"请选择一个几何对象进行预览。"));
    m_detailLabel->setText(zh(u8"VTK 渲染窗口已就绪。"));
    if (m_canvas) {
        m_canvas->showEmpty();
    }
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
    m_detailLabel->setText(zh(u8"根据长方体参数生成的 VTK 立方体预览。"));
    if (m_canvas) {
        m_canvas->showBoxGeometry(box);
    }
}

void RenderView::showOccShape(const TopoDS_Shape &shape, const QString &title, const QString &subtitle)
{
    m_titleLabel->setText(title);
    m_subtitleLabel->setText(subtitle);
    m_detailLabel->setText(zh(u8"Open CASCADE 形状已转换为 VTK 多边形数据。"));
    if (m_canvas) {
        m_canvas->showOccShape(shape, title);
    }
}

void RenderView::showMeshGrid(vtkSmartPointer<vtkUnstructuredGrid> grid, const QString &title, const QString &subtitle)
{
    m_titleLabel->setText(title);
    m_subtitleLabel->setText(subtitle);
    m_detailLabel->setText(zh(u8"Gmsh 四面体网格已转换为 VTK 非结构网格。"));
    if (m_canvas) {
        m_canvas->showMeshGrid(grid);
    }
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
    m_detailLabel->setText(zh(u8"CalculiX 标量场：%1 [%2, %3] %4")
        .arg(scalarName)
        .arg(scalarMin, 0, 'g', 6)
        .arg(scalarMax, 0, 'g', 6)
        .arg(scalarUnit));
    if (!m_canvas) {
        return;
    }
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
    if (m_canvas) {
        m_canvas->setPickMode(mode);
    }
}

void RenderView::clearHighlight()
{
    if (m_canvas) {
        m_canvas->clearHighlight();
    }
}

void RenderView::highlightFaceGroup(const FaceGroup &faceGroup)
{
    if (m_canvas) {
        m_canvas->highlightFaceGroup(faceGroup);
    }
}

void RenderView::highlightFaceIndices(const std::vector<int> &faceIndices)
{
    if (m_canvas) {
        m_canvas->highlightFaceIndices(faceIndices);
    }
}

void RenderView::highlightResultPosition(double x, double y, double z)
{
    if (m_canvas) {
        m_canvas->highlightResultPosition(x, y, z);
    }
}

void RenderView::highlightResultExtrema(const ResultExtremeMarker &minimum, const ResultExtremeMarker &maximum)
{
    if (m_canvas) {
        m_canvas->highlightResultExtrema(minimum, maximum);
    }
}

QString RenderView::activeGeometryName() const
{
    return m_canvas ? m_canvas->activeGeometryName() : QString();
}
