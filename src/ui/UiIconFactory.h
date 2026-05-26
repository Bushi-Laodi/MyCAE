#pragma once

#include <QColor>
#include <QIcon>
#include <QString>

class UiIconFactory
{
public:
    static QIcon treeBadge(const QString &text, const QColor &background, const QColor &foreground = Qt::white);
    static QIcon toolbarBadge(const QString &text, const QColor &background, const QColor &foreground = Qt::white);
};
