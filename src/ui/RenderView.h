#pragma once

#include <QFrame>

class QLabel;
class VtkRenderCanvas;
struct BoxGeometry;

class RenderView final : public QFrame
{
public:
    explicit RenderView(QWidget *parent = nullptr);

    void showEmpty();
    void showBoxGeometry(const BoxGeometry &box);

private:
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QLabel *m_detailLabel = nullptr;
    VtkRenderCanvas *m_canvas = nullptr;
};
