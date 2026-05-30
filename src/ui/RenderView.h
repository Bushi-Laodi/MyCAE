#pragma once

#include "picking/PickMode.h"
#include "picking/PickSelection.h"
#include "render/VtkHighlightActorFactory.h"
#include "result/ResultExtrema.h"
#include "result/ResultProbe.h"
#include "ui/RenderGeometryItem.h"

#include <QFrame>
#include <QString>
#include <vtkSmartPointer.h>

#include <vector>

class QLabel;
class TopoDS_Shape;
class VtkRenderCanvas;
class vtkUnstructuredGrid;
struct BoxGeometry;
struct FaceGroup;

class RenderView final : public QFrame
{
    Q_OBJECT

public:
    explicit RenderView(QWidget *parent = nullptr);

    void showEmpty();
    void showBoxGeometry(const BoxGeometry &box);
    void showOccShape(const TopoDS_Shape &shape, const QString &title, const QString &subtitle);
    void showGeometryScene(
        const std::vector<RenderGeometryItem> &items,
        const QString &selectedGeometryName,
        const QString &subtitle,
        bool resetCamera = true
    );
    void showMeshGrid(vtkSmartPointer<vtkUnstructuredGrid> grid, const QString &title, const QString &subtitle);
    void setMeshTransparent(bool transparent);
    bool meshTransparent() const;
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
        const ResultExtremeMarker &minimumMarker,
        const ResultExtremeMarker &maximumMarker,
        bool resetCamera = true
    );
    void setGeometryEdgesVisible(bool visible);
    bool geometryEdgesVisible() const;
    void setOrientationMarkerVisible(bool visible);
    bool orientationMarkerVisible() const;
    bool saveScreenshot(const QString &filePath);
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
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QLabel *m_detailLabel = nullptr;
    VtkRenderCanvas *m_canvas = nullptr;
};
