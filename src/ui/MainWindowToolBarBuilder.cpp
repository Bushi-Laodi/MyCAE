#include "ui/MainWindowToolBarBuilder.h"

#include "ui/MainWindowActions.h"

#include <QAction>
#include <QColor>
#include <QFont>
#include <QIcon>
#include <QMainWindow>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QStyle>
#include <QToolBar>

namespace
{
QIcon badgeIcon(const QString &text, const QColor &background, const QColor &foreground = Qt::white)
{
    constexpr int size = 32;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(background);
    painter.drawRoundedRect(3, 3, size - 6, size - 6, 6, 6);

    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(text.size() > 1 ? 10 : 13);
    painter.setFont(font);
    painter.setPen(foreground);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);
    return QIcon(pixmap);
}

void prepareToolAction(QAction *action, const QIcon &icon, const QString &toolTip)
{
    if (!action) {
        return;
    }
    action->setIcon(icon);
    action->setToolTip(toolTip);
}
}

void MainWindowToolBarBuilder::build(QMainWindow *window, const MainWindowActions &actions)
{
    auto *toolBar = window->addToolBar("Main Toolbar");
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setIconSize(QSize(20, 20));
    toolBar->setStyleSheet(
        "QToolBar {"
        "  spacing: 4px;"
        "  padding: 3px 6px;"
        "  border: 0;"
        "  border-bottom: 1px solid #d8dce2;"
        "  background: #f6f7f9;"
        "}"
        "QToolButton {"
        "  padding: 4px;"
        "  border: 1px solid transparent;"
        "  border-radius: 4px;"
        "}"
        "QToolButton:hover {"
        "  background: #e8eef7;"
        "  border-color: #c8d5e8;"
        "}"
        "QToolButton:checked {"
        "  background: #dbeafe;"
        "  border-color: #8db7f0;"
        "}"
        "QToolButton:disabled {"
        "  background: transparent;"
        "}"
    );

    prepareToolAction(
        actions.newProject,
        window->style()->standardIcon(QStyle::SP_FileIcon),
        "New Project"
    );
    prepareToolAction(
        actions.openProject,
        window->style()->standardIcon(QStyle::SP_DialogOpenButton),
        "Open Project"
    );
    prepareToolAction(actions.createBox, badgeIcon("B", QColor("#2563eb")), "Create Box");
    prepareToolAction(actions.createCylinder, badgeIcon("C", QColor("#0f766e")), "Create Cylinder");
    prepareToolAction(actions.pickFace, badgeIcon("P", QColor("#7c3aed")), "Pick Face");
    prepareToolAction(
        actions.createFaceGroupFromPick,
        badgeIcon("G+", QColor("#15803d")),
        "Create Face Group from Pick"
    );
    prepareToolAction(
        actions.addPickedFacesToFaceGroup,
        badgeIcon("F+", QColor("#ca8a04")),
        "Add Picked Faces to Face Group"
    );
    prepareToolAction(
        actions.removePickedFacesFromFaceGroup,
        badgeIcon("F-", QColor("#dc2626")),
        "Remove Picked Faces from Face Group"
    );

    toolBar->addAction(actions.newProject);
    toolBar->addAction(actions.openProject);
    toolBar->addSeparator();
    toolBar->addAction(actions.createBox);
    toolBar->addAction(actions.createCylinder);
    toolBar->addSeparator();
    toolBar->addAction(actions.pickFace);
    toolBar->addAction(actions.createFaceGroupFromPick);
    toolBar->addAction(actions.addPickedFacesToFaceGroup);
    toolBar->addAction(actions.removePickedFacesFromFaceGroup);
}
