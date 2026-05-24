/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**********************************************************************
 * @file   BoundaryDeleteDialog.h
 * @brief  边界参数删除对话框
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-08-26
 *********************************************************************/
#ifndef _BoundaryDeleteDialog_H
#define _BoundaryDeleteDialog_H

#include "GUIWidget/GUIDialogBase.h"
#include "GUICalculateDialogAPI.h"

namespace Ui {
    class BoundaryDeleteDialog;
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
     * @brief  边界参数创建对话框
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-08-26
     */
    class GUICalculateDialogAPI BoundaryDeleteDialog : public GUIDialogBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @param[i] oper 操作器对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        BoundaryDeleteDialog(Interface::FITKOFBoundary* obj,EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief    析构函数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        ~BoundaryDeleteDialog();
    private slots:
        ;
        /**
         * @brief Ok点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void on_pushButton_OK_clicked();
        /**
         * @brief 取消点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void on_pushButton_Cancel_clicked();
    private:
        /**
         * @brief  ui界面
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-28
         */
        Ui::BoundaryDeleteDialog* _ui = nullptr;
        /**
         * @brief  边界对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-28
         */
        Interface::FITKOFBoundary* _obj = nullptr;
        /**
         * @brief  操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-28
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
