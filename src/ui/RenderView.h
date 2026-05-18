#pragma once

#include <QFrame>
#include <QString>
#include <vtkSmartPointer.h>

class QLabel;
class TopoDS_Shape;
class VtkRenderCanvas;
class vtkUnstructuredGrid;
struct BoxGeometry;

class RenderView final : public QFrame
{
public:
    explicit RenderView(QWidget *parent = nullptr);

    void showEmpty();
    void showBoxGeometry(const BoxGeometry &box);
    void showOccShape(const TopoDS_Shape &shape, const QString &title, const QString &subtitle);
    void showMeshGrid(vtkSmartPointer<vtkUnstructuredGrid> grid, const QString &title, const QString &subtitle);

private:
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QLabel *m_detailLabel = nullptr;
    VtkRenderCanvas *m_canvas = nullptr;
};
