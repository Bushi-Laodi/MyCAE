#include "ui/UiIconFactory.h"

#include <QFont>
#include <QPainter>
#include <QPixmap>

namespace
{
QIcon badgeIcon(
    const QString &text,
    const QColor &background,
    const QColor &foreground,
    int size,
    int margin,
    int radius,
    int singleCharacterPointSize,
    int multiCharacterPointSize
)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(background);
    painter.drawRoundedRect(margin, margin, size - margin * 2, size - margin * 2, radius, radius);

    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(text.size() > 1 ? multiCharacterPointSize : singleCharacterPointSize);
    painter.setFont(font);
    painter.setPen(foreground);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);
    return QIcon(pixmap);
}
}

QIcon UiIconFactory::treeBadge(const QString &text, const QColor &background, const QColor &foreground)
{
    return badgeIcon(text, background, foreground, 18, 1, 4, 9, 7);
}

QIcon UiIconFactory::toolbarBadge(const QString &text, const QColor &background, const QColor &foreground)
{
    return badgeIcon(text, background, foreground, 32, 3, 6, 13, 10);
}
