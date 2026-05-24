/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**********************************************************************
 * @file   BoundaryCreateDialog.h
 * @brief  边界参数创建对话框
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-08-26
 *********************************************************************/
#ifndef _BoundaryCreateDialog_H
#define _BoundaryCreateDialog_H

#include "GUIWidget/GUIDialogBase.h"
#include "GUICalculateDialogAPI.h"

namespace Ui {
    class BoundaryCreateDialog;
}

namespace Interface {
    class FITKOFPhysicsData;
    class FITKFlowPhysicsHandlerFactory;
    class FITKUnstructuredFluidMeshVTK;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief  边界参数创建对话框
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-08-26
     */
    class GUICalculateDialogAPI BoundaryCreateDialog : public GUIDialogBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @param[i] oper 操作器对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        BoundaryCreateDialog(EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief    析构函数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        ~BoundaryCreateDialog();
        /**
         * @brief    初始化
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void init();
    protected:
        /**
         * @brief    隐藏事件重写
         * @param[i] event 事件对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void hideEvent(QHideEvent *event) override;
        /**
         * @brief    显示事件重写
         * @param[i] event 事件对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void showEvent(QShowEvent *event) override;
    private slots:
        ;
        void on_comboBox_Boundary_activated(int index);

        void on_comboBox_MeshRegion_activated(int index);

        void on_comboBox_InterBoundary_activated(int index);

        void on_comboBox_InterMeshRegion_activated(int index);

        void on_comboBox_Type_activated(int index);

        void on_pushButton_OK_clicked();

        void on_pushButton_Cancel_clicked();
    private:
        /**
         * @brief    初始化边界下拉框
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-23
         */
        void initBoundaryComboBox();
        /**
         * @brief    初始化区域接口边界下拉框
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-07-04
         */
        void initInterfaceBoundaryComboBox();
        /**
         * @brief    初始化区域接口下拉框
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-07-04
         */
        void initInterfaceMeshRegionComboBox();

        /**
         * @brief    高亮网格边界
         * @param[i] meshBoundID   网格边界id
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void highlightMeshBoundary(int meshBoundID);
    private:
        /**
         * @brief  ui
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        Ui::BoundaryCreateDialog* _ui = nullptr;
        /**
         * @brief  操作器
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief  物理数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-21
         */
        Interface::FITKOFPhysicsData* _physicsData = nullptr;
        /**
         * @brief  工厂对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-21
         */
        Interface::FITKFlowPhysicsHandlerFactory* _factoryData = nullptr;
        /**
         * @brief  网格数据
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date   2025-06-23
         */
        Interface::FITKUnstructuredFluidMeshVTK* _meshData = nullptr;
    };
}

#endif
