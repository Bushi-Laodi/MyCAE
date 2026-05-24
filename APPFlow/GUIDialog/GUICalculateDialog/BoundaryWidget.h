/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file BoundaryWidget.h
 * @brief 求解器参数边界界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _BoundaryWidget_H
#define _BoundaryWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFEnum.hpp"

namespace Ui {
    class BoundaryWidget;
}

namespace Interface {
    class FITKOFBoundary;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief 求解器参数边界界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI BoundaryWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Boundary Widget object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        BoundaryWidget(Interface::FITKOFBoundary* boundaryObj, EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Boundary Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~BoundaryWidget();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief    更新
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-27
         */
        void update();
        /**
         * @brief    获取当前的数据对象
         * @return   Interface::FITKOFBoundary* 当前数据对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-28
         */
        Interface::FITKOFBoundary* getCurrentObj();
    protected:
        /**
         * @brief    显示事件
         * @param[i] event 事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-27
         */
        void showEvent(QShowEvent *event) override;
        /**
         * @brief    隐藏事件
         * @param[i] event 事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-27
         */
        void hideEvent(QHideEvent *event) override;
    private:
        /**
         * @brief    更新Flow
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-28
         */
        void updateFlow();
        /**
         * @brief    更新湍流
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-28
         */
        void updateTurbulence();
        /**
         * @brief    更新phases
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-28
         */
        void updatePhases();
        /**
         * @brief    更新Thermal
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-23
         */
        void updateThermal();
    private:
        /**
         * @brief ui
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
         Ui::BoundaryWidget* _ui = nullptr;
         /**
          * @brief  边界数据对象
          * @author BaGuijun (baguijun@163.com)
          * @date   2024-08-27
          */
         Interface::FITKOFBoundary* _boundaryObj = nullptr;
         /**
          * @brief  当前边界类型
          * @author BaGuijun (baguijun@163.com)
          * @date   2024-08-27
          */
         Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType _currentType = Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BNone;
    };
}
#endif
