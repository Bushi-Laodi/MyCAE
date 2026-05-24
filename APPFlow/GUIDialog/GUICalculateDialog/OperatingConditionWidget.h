/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file OperatingConditionWidget.h
 * @brief 求解器运行条件设置界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatingConditionWidget_H
#define _OperatingConditionWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

namespace Ui {
    class OperatingConditionWidget;
}

namespace Interface {
    class FITKOFInitialConditions;
    class FITKOFOperatingConditions;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class InitialCreatePatchDialog;
    /**
     * @brief 求解器运行条件环境界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI OperatingConditionWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Initial Widget object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatingConditionWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Initial Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatingConditionWidget();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief    更新界面
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-05
         */
        void updateWidget();
    private:
        /**
         * @brief  ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-05
         */
        Ui::OperatingConditionWidget* _ui = nullptr;
        /**
         * @brief  运行条件数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-05
         */
        Interface::FITKOFOperatingConditions* _operCondition = nullptr;
    };
}
#endif
