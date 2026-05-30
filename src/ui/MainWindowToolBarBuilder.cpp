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

void configureToolBar(QToolBar *toolBar)
{
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setIconSize(QSize(20, 20));
}
}

void MainWindowToolBarBuilder::build(QMainWindow *window, const MainWindowActions &actions)
{
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
    prepareToolAction(actions.createBoolean, UiIconFactory::toolbarBadge("B∩", QColor("#7c3aed")), zh(u8"布尔操作"));
    prepareToolAction(actions.transformGeometry, UiIconFactory::toolbarBadge("T", QColor("#ea580c")), zh(u8"变换选中几何体"));
    prepareToolAction(actions.createStructuralMaterial, UiIconFactory::toolbarBadge("MAT", QColor("#0f766e")), zh(u8"创建结构材料"));
    prepareToolAction(actions.createSectionAssignment, UiIconFactory::toolbarBadge("SA", QColor("#16a34a")), zh(u8"创建材料分区"));
    prepareToolAction(actions.createStructuralBoundaryCondition, UiIconFactory::toolbarBadge("BC", QColor("#2563eb")), zh(u8"创建结构约束"));
    prepareToolAction(actions.createStructuralLoad, UiIconFactory::toolbarBadge("F", QColor("#dc2626")), zh(u8"创建结构载荷"));
    prepareToolAction(actions.generateMesh, UiIconFactory::toolbarBadge("M", QColor("#0891b2")), zh(u8"生成网格"));
    prepareToolAction(actions.readMeshInfo, UiIconFactory::toolbarBadge("Q", QColor("#4b5563")), zh(u8"读取网格信息"));
    prepareToolAction(actions.showMesh, UiIconFactory::toolbarBadge("Ms", QColor("#0f766e")), zh(u8"显示网格"));
    prepareToolAction(actions.clearPick, UiIconFactory::toolbarBadge("PX", QColor("#6b7280")), zh(u8"清除拾取"));
    prepareToolAction(actions.setFaceGroupLocalMeshSize, UiIconFactory::toolbarBadge("h", QColor("#0284c7")), zh(u8"设置面组局部网格尺寸"));
    prepareToolAction(actions.showGeometryEdges, UiIconFactory::toolbarBadge("E", QColor("#475569")), zh(u8"显示几何边线"));
    prepareToolAction(actions.showOrientationMarker, UiIconFactory::toolbarBadge("XYZ", QColor("#334155")), zh(u8"显示或隐藏 XYZ 坐标轴"));
    prepareToolAction(actions.showMeshTransparent, UiIconFactory::toolbarBadge("α", QColor("#64748b")), zh(u8"主模型半透明"));
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
    for (QAction *runSolverAction : actions.runSolvers) {
        prepareToolAction(runSolverAction, UiIconFactory::toolbarBadge("R", QColor("#16a34a")), runSolverAction->text());
    }

    auto *workflowToolBar = window->addToolBar(zh(u8"主流程工具栏"));
    configureToolBar(workflowToolBar);
    workflowToolBar->addAction(actions.newProject);
    workflowToolBar->addAction(actions.openProject);
    workflowToolBar->addSeparator();
    workflowToolBar->addAction(actions.importStep);
    workflowToolBar->addAction(actions.createBox);
    workflowToolBar->addAction(actions.createCylinder);
    workflowToolBar->addAction(actions.createSphere);
    workflowToolBar->addAction(actions.createBoolean);
    workflowToolBar->addAction(actions.transformGeometry);
    workflowToolBar->addSeparator();
    workflowToolBar->addAction(actions.createStructuralMaterial);
    workflowToolBar->addAction(actions.createSectionAssignment);
    workflowToolBar->addAction(actions.createStructuralBoundaryCondition);
    workflowToolBar->addAction(actions.createStructuralLoad);
    workflowToolBar->addSeparator();
    workflowToolBar->addAction(actions.generateMesh);
    workflowToolBar->addAction(actions.readMeshInfo);
    workflowToolBar->addAction(actions.showMesh);
    if (!actions.runSolvers.isEmpty()) {
        workflowToolBar->addSeparator();
        for (QAction *runSolverAction : actions.runSolvers) {
            workflowToolBar->addAction(runSolverAction);
        }
    }

    auto *selectionToolBar = window->addToolBar(zh(u8"选择与显示工具栏"));
    configureToolBar(selectionToolBar);
    selectionToolBar->addAction(actions.pickFace);
    selectionToolBar->addAction(actions.clearPick);
    selectionToolBar->addAction(actions.createFaceGroupFromPick);
    selectionToolBar->addAction(actions.addPickedFacesToFaceGroup);
    selectionToolBar->addAction(actions.removePickedFacesFromFaceGroup);
    selectionToolBar->addAction(actions.setFaceGroupLocalMeshSize);
    selectionToolBar->addSeparator();
    selectionToolBar->addAction(actions.showGeometryEdges);
    selectionToolBar->addAction(actions.showOrientationMarker);
    selectionToolBar->addAction(actions.showMeshTransparent);
}
