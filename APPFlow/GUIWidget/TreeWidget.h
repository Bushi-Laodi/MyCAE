/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @brief 定义模型树点击事件处理器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-06
 */
#ifndef _TreeWidget_H__
#define _TreeWidget_H__

#include <QTreeWidget>
#include "GUIWidgetAPI.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace GUI {

    class ModelCaseTree;
    /**
     * @brief 定义模型树点击事件处理器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-06
     */
    class GUIWIDGETAPI TreeWidget : public QTreeWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Tree Widget object
         * @param[i]  parent         My Param doc
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        TreeWidget(QWidget* parent);
        /**
         * @brief Destroy the Tree Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        ~TreeWidget();
        /**
         * @brief 更新树
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void updateTree();
    private slots:
        ;
        /**
         * @brief 鼠标右键点击事件
         * @param[i]  point          点击位置
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void onModelCustomContextMenu(QPoint point);
        /**
         * @brief 鼠标点击事件
         * @param[i]  item           点击对象
         * @param[i]  column         点击列数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void onItemClicked(QTreeWidgetItem *item, int column);
        /**
         * @brief 鼠标双击事件
         * @param[i]  item           点击对象
         * @param[i]  column         点击列数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void onDoubleClicked(QTreeWidgetItem *item, int column);
        /**
         * @brief 鼠标又右键按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void acitonClicked();
        /**
         * @brief 树节点图标按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void soltIconButtonClicked();
    private:
        /**
         * @brief 更新几何子节点
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void updateGeometryItems();
        /**
         * @brief 更新网格子节点
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void updateMeshItems();
        /**
         * @brief    更新求解器参数节点
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        void updateSetupItems();
        /**
         * @brief    更新求解器运行节点
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        void updateRunItems();
        /**
         * @brief    更新后处理节点
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        void updatePostItems();
        /**
         * @brief 更新local子节点
         * @param[i]  parentItem     父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void updateMeshLocalItems(QTreeWidgetItem* parentItem);
        /**
         * @brief 更新boundary子节点
         * @param[i]  parentItem     父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void updateMeshBoundaryItems(QTreeWidgetItem* parentItem);
        /**
         * @brief 更新Setup子节点
         * @param[i]  parentItem     父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void updateSetupItems(QTreeWidgetItem* parentItem);
        /**
         * @brief 添加action
         * @param[i]  menu           My Param doc
         * @param[i]  actions        My Param doc
         * @param[i]  objectName     My Param doc
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-06
         */
        void addMenuActions(QMenu& menu, QString actions, QString objectName);
    };
}  

#endif
