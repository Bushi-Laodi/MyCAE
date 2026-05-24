/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file MeshGeoWidget.h
 * @brief 几何相关的网格区域尺寸界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _MeshGeoWidget_H
#define _MeshGeoWidget_H

#include "GUIMeshDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"

class QVBoxLayout;

namespace Ui {
    class MeshGeoWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief 几何相关的网格区域尺寸界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIMeshDialogAPI MeshGeoWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Mesh Geo Widget object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshGeoWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Mesh Geo Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~MeshGeoWidget();
        /**
         * @brief 初始化界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief    更新界面
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-03
         */
        void updateWidget();
    private:
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief  ui
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-02
         */
        Ui::MeshGeoWidget* _ui = nullptr;
    };
}

#endif
