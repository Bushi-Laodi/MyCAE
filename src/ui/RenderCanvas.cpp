#include "RenderCanvas.h"

#include <algorithm>
#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QSizePolicy>
#include <QString>

RenderCanvas::RenderCanvas(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void RenderCanvas::showEmpty()
{
    m_hasBox = false;
    update();
}

void RenderCanvas::showBoxGeometry(const BoxGeometry &box)
{
    m_hasBox = true;
    m_currentBox = box;
    update();
}

void RenderCanvas::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (m_hasBox) {
        drawBoxWireframe();
    }
}

void RenderCanvas::drawBoxWireframe()
{
    const double length = std::max(0.001, m_currentBox.length);
    const double width = std::max(0.001, m_currentBox.width);
    const double height = std::max(0.001, m_currentBox.height);

    QRectF area = rect().adjusted(56, 24, -56, -44);
    if (area.width() < 120 || area.height() < 120) {
        area = rect().adjusted(24, 16, -24, -32);
    }
    if (area.width() <= 1 || area.height() <= 1) {
        return;
    }

    const double depthXRatio = 0.45;
    const double depthYRatio = 0.32;
    const double projectedWidth = length + width * depthXRatio;
    const double projectedHeight = height + width * depthYRatio;
    const double scale = std::min(area.width() / projectedWidth, area.height() / projectedHeight);
    if (scale <= 0.0) {
        return;
    }

    const double frontWidth = length * scale;
    const double frontHeight = height * scale;
    const double offsetX = width * scale * depthXRatio;
    const double offsetY = -width * scale * depthYRatio;
    const double totalWidth = frontWidth + offsetX;
    const double totalHeight = frontHeight - offsetY;

    const double left = area.center().x() - totalWidth / 2.0;
    const double top = area.center().y() - totalHeight / 2.0;

    const QPointF frontTopLeft(left, top - offsetY);
    const QPointF frontTopRight(frontTopLeft.x() + frontWidth, frontTopLeft.y());
    const QPointF frontBottomLeft(frontTopLeft.x(), frontTopLeft.y() + frontHeight);
    const QPointF frontBottomRight(frontTopRight.x(), frontTopRight.y() + frontHeight);

    const QPointF backTopLeft(frontTopLeft.x() + offsetX, frontTopLeft.y() + offsetY);
    const QPointF backTopRight(frontTopRight.x() + offsetX, frontTopRight.y() + offsetY);
    const QPointF backBottomLeft(frontBottomLeft.x() + offsetX, frontBottomLeft.y() + offsetY);
    const QPointF backBottomRight(frontBottomRight.x() + offsetX, frontBottomRight.y() + offsetY);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen backPen(QColor("#6f7c89"));
    backPen.setWidthF(1.4);
    backPen.setStyle(Qt::DashLine);
    painter.setPen(backPen);
    painter.drawLine(backTopLeft, backTopRight);
    painter.drawLine(backTopRight, backBottomRight);
    painter.drawLine(backBottomRight, backBottomLeft);
    painter.drawLine(backBottomLeft, backTopLeft);

    QPen edgePen(QColor("#d7dde5"));
    edgePen.setWidthF(2.0);
    painter.setPen(edgePen);
    painter.drawLine(frontTopLeft, frontTopRight);
    painter.drawLine(frontTopRight, frontBottomRight);
    painter.drawLine(frontBottomRight, frontBottomLeft);
    painter.drawLine(frontBottomLeft, frontTopLeft);
    painter.drawLine(frontTopLeft, backTopLeft);
    painter.drawLine(frontTopRight, backTopRight);
    painter.drawLine(frontBottomLeft, backBottomLeft);
    painter.drawLine(frontBottomRight, backBottomRight);

    QPen guidePen(QColor("#9ba7b4"));
    guidePen.setWidthF(1.2);
    painter.setPen(guidePen);
    painter.drawText(QRectF(area.left(), rect().bottom() - 28, area.width(), 24),
                     Qt::AlignCenter,
                     QString("长 %1 %4   宽 %2 %4   高 %3 %4")
                         .arg(m_currentBox.length)
                         .arg(m_currentBox.width)
                         .arg(m_currentBox.height)
                         .arg(m_currentBox.unit));
}
