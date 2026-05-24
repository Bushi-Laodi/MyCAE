/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**********************************************************************
 * @file   MeshInfoWidget.h
 * @brief  网格信息界面
 * @author liuzhonghua (liuzhonghuaszch@163.com)
 * @date   2025-06-13
 *********************************************************************/
#ifndef _MeshInfoWidget_H
#define _MeshInfoWidget_H

#include "GUIMeshDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

class QTableWidgetItem;

namespace Ui{
    class MeshInfoWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class CompFaceGroupWidget;
    /**
     * @brief cude create or edit widget
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIMeshDialogAPI MeshInfoWidget :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief create constructor
         * @param[i]  oper           operators
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Mesh Info Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~MeshInfoWidget();
        /**
         * @brief init
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
    protected:
        /**
         * @brief close event override
         * @param[i]  event          event object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void closeEvent(QCloseEvent *event) override;

    private:

    private:
        /**
         * @brief ui object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::MeshInfoWidget* _ui = nullptr;
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
