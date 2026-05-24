/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**********************************************************************
 * @file   CompTranPhasesWidget.h
 * @brief  材料阶段组件界面
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-08-26
 *********************************************************************/
#ifndef _CompTranPhasesWidget_H
#define _CompTranPhasesWidget_H

#include "GUICalculateWidgetBase.h"
#include "GUICalculateDialogAPI.h"

namespace Ui {
    class CompTranPhasesWidget;
}

namespace Interface {
    class FITKOFTransportPhase;
}

namespace GUI
{
    /**
     * @brief  材料阶段组件界面
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-08-26
     */
    class GUICalculateDialogAPI CompTranPhasesWidget : public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @param[i] phaseData  材料阶段数据对象
         * @param[i] parent     父对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        CompTranPhasesWidget(Interface::FITKOFTransportPhase* phaseData, int index, QWidget* parent = nullptr);
        /**
         * @brief    析构函数
         * @return   void
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        ~CompTranPhasesWidget();
        /**
         * @brief    初始化
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void init();
    private:
        /**
         * @brief  ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        Ui::CompTranPhasesWidget* _ui = nullptr;
        /**
         * @brief  索引值
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        int _index = -1;
        /**
         * @brief  材料阶段对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        Interface::FITKOFTransportPhase* _phaseData = nullptr;
    };
}

#endif
