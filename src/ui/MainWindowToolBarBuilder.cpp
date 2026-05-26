#include "ui/MainWindowToolBarBuilder.h"

#include "ui/MainWindowActions.h"
#include "ui/UiIconFactory.h"

#include <QAction>
#include <QColor>
#include <QIcon>
#include <QMainWindow>
#include <QSize>
#include <QStyle>
#include <QToolBar>

namespace
{
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
    prepareToolAction(actions.createBox, UiIconFactory::toolbarBadge("B", QColor("#2563eb")), "Create Box");
    prepareToolAction(actions.createCylinder, UiIconFactory::toolbarBadge("C", QColor("#0f766e")), "Create Cylinder");
    prepareToolAction(actions.pickFace, UiIconFactory::toolbarBadge("P", QColor("#7c3aed")), "Pick Face");
    prepareToolAction(
        actions.createFaceGroupFromPick,
        UiIconFactory::toolbarBadge("G+", QColor("#15803d")),
        "Create Face Group from Pick"
    );
    prepareToolAction(
        actions.addPickedFacesToFaceGroup,
        UiIconFactory::toolbarBadge("F+", QColor("#ca8a04")),
        "Add Picked Faces to Face Group"
    );
    prepareToolAction(
        actions.removePickedFacesFromFaceGroup,
        UiIconFactory::toolbarBadge("F-", QColor("#dc2626")),
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
