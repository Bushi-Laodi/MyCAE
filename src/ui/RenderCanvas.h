#pragma once

#include "geometry/BoxGeometry.h"

#include <QWidget>

class QPaintEvent;

class RenderCanvas final : public QWidget
{
public:
    explicit RenderCanvas(QWidget *parent = nullptr);

    void showEmpty();
    void showBoxGeometry(const BoxGeometry &box);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawBoxWireframe();

    bool m_hasBox = false;
    BoxGeometry m_currentBox;
};
