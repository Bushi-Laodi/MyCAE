#pragma once

#include "picking/PickMode.h"
#include "picking/PickSelection.h"

#include <QFrame>
#include <QString>
#include <vtkSmartPointer.h>

#include <vector>

class QLabel;
class TopoDS_Shape;
class VtkRenderCanvas;
class vtkUnstructuredGrid;
struct BoxGeometry;

class RenderView final : public QFrame
{
    Q_OBJECT

public:
    explicit RenderView(QWidget *parent = nullptr);

    void showEmpty();
    void showBoxGeometry(const BoxGeometry &box);
    void showOccShape(const TopoDS_Shape &shape, const QString &title, const QString &subtitle);
    void showMeshGrid(vtkSmartPointer<vtkUnstructuredGrid> grid, const QString &title, const QString &subtitle);
    void showResultGrid(
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
        bool resetCamera = true
    );
    bool saveScreenshot(const QString &filePath);
    void setPickMode(PickMode mode);
    void clearHighlight();
    void highlightFaceIndices(const std::vector<int> &faceIndices);
    void highlightResultPosition(double x, double y, double z);

signals:
    void facePicked(const PickSelection &selection);

private:
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QLabel *m_detailLabel = nullptr;
    VtkRenderCanvas *m_canvas = nullptr;
};
