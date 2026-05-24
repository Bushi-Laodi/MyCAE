/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file GeometryDeleteDialog.h
 * @brief 几何删除对话框
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef GeometryDeleteDialog_H
#define GeometryDeleteDialog_H

#include "GUIGeometryDialogAPI.h"
#include "GUIWidget/GUIDialogBase.h"

namespace Ui {
    class GeometryDeleteDialog;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKAbsGeoCommand;
}

namespace GUI 
{    
    /**
     * @brief 几何删除对话框
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIGeometryDialogAPI GeometryDeleteDialog : public GUIDialogBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Geometry Delete Dialog object
         * @param[i]  obj            几何对象
         * @param[i]  oper           操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        GeometryDeleteDialog(Interface::FITKAbsGeoCommand* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Geometry Delete Dialog object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~GeometryDeleteDialog();
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
    protected:
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::GeometryDeleteDialog* _ui = nullptr;
        /**
         * @brief 几何数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKAbsGeoCommand* _obj = nullptr;
    };
}

#endif
