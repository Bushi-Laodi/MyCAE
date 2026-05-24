/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file LocalSelectGroupWidget.h
 * @brief 面组选择界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _LocalSelectGroupWidget_H
#define _LocalSelectGroupWidget_H

#include "GUIWidget/GUIWidgetBase.h"
#include "GUIMeshDialogAPI.h"

namespace Ui {
    class LocalSelectGroupWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief 面组选择界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIMeshDialogAPI LocalSelectGroupWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Local Select Group Widget object
         * @param[i]  oper           操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        LocalSelectGroupWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Local Select Group Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~LocalSelectGroupWidget();
        /**
         * @brief 初始化参数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief 更新表格界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void updateTableWidget();
    private slots:
        ;
        /**
         * @brief 添加按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_Add_clicked();
    private:
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::LocalSelectGroupWidget* _ui = nullptr;
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };

}

#endif
