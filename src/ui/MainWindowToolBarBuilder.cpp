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
QString zh(const char *text)
{
    return QString::fromUtf8(text);
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
    auto *toolBar = window->addToolBar(zh(u8"主工具栏"));
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setIconSize(QSize(20, 20));

    prepareToolAction(
        actions.newProject,
        window->style()->standardIcon(QStyle::SP_FileIcon),
        zh(u8"新建工程")
    );
    prepareToolAction(
        actions.openProject,
        window->style()->standardIcon(QStyle::SP_DialogOpenButton),
        zh(u8"打开工程")
    );
    prepareToolAction(actions.createBox, UiIconFactory::toolbarBadge("B", QColor("#2563eb")), zh(u8"创建长方体"));
    prepareToolAction(actions.createCylinder, UiIconFactory::toolbarBadge("C", QColor("#0f766e")), zh(u8"创建圆柱体"));
    prepareToolAction(actions.createSphere, UiIconFactory::toolbarBadge("S", QColor("#9333ea")), zh(u8"创建球体"));
    prepareToolAction(actions.importStep, UiIconFactory::toolbarBadge("I", QColor("#475569")), zh(u8"导入 STEP"));
    prepareToolAction(actions.transformGeometry, UiIconFactory::toolbarBadge("T", QColor("#ea580c")), zh(u8"变换选中几何体"));
    prepareToolAction(actions.deleteGeometry, UiIconFactory::toolbarBadge("D", QColor("#dc2626")), zh(u8"删除选中几何体"));
    prepareToolAction(actions.showOrientationMarker, UiIconFactory::toolbarBadge("XYZ", QColor("#334155")), zh(u8"显示或隐藏 XYZ 坐标轴"));
    prepareToolAction(actions.pickFace, UiIconFactory::toolbarBadge("P", QColor("#7c3aed")), zh(u8"拾取面"));
    prepareToolAction(
        actions.createFaceGroupFromPick,
        UiIconFactory::toolbarBadge("G+", QColor("#15803d")),
        zh(u8"从拾取创建面组")
    );
    prepareToolAction(
        actions.addPickedFacesToFaceGroup,
        UiIconFactory::toolbarBadge("F+", QColor("#ca8a04")),
        zh(u8"将拾取面加入面组")
    );
    prepareToolAction(
        actions.removePickedFacesFromFaceGroup,
        UiIconFactory::toolbarBadge("F-", QColor("#dc2626")),
        zh(u8"从面组移除拾取面")
    );

    toolBar->addAction(actions.newProject);
    toolBar->addAction(actions.openProject);
    toolBar->addSeparator();
    toolBar->addAction(actions.createBox);
    toolBar->addAction(actions.createCylinder);
    toolBar->addAction(actions.createSphere);
    toolBar->addAction(actions.importStep);
    toolBar->addAction(actions.transformGeometry);
    toolBar->addAction(actions.deleteGeometry);
    toolBar->addSeparator();
    toolBar->addAction(actions.showOrientationMarker);
    toolBar->addSeparator();
    toolBar->addAction(actions.pickFace);
    toolBar->addAction(actions.createFaceGroupFromPick);
    toolBar->addAction(actions.addPickedFacesToFaceGroup);
    toolBar->addAction(actions.removePickedFacesFromFaceGroup);
}
