/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file LocalGroupInfoWidget.h
 * @brief 网格尺寸编辑界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef LocalGroupInfoWidget_H
#define LocalGroupInfoWidget_H

#include "GUIWidget/GUIWidgetBase.h"
#include "GUIMeshDialogAPI.h"

namespace Ui {
    class LocalGroupInfoWidget;
}

namespace Interface {
    class FITKGeometryMeshSize;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief 网格尺寸编辑界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIMeshDialogAPI LocalGroupInfoWidget : public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Local Group Info Widget object
         * @param[i]  obj            网格尺寸对象
         * @param[i]  oper           操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        LocalGroupInfoWidget(Interface::FITKGeometryMeshSize* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Local Group Info Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~LocalGroupInfoWidget();
        /**
         * @brief 初始化界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief 获取当前网格尺寸对象
         * @return Interface::FITKGeometryMeshSize*  网格尺寸对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKGeometryMeshSize* getCurrentGeoMeshObj();
    private slots:
        ;
        /**
         * @brief 边界层数编辑事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_lineEdit_NoLayers_editingFinished();
        /**
         * @brief 膨胀率编辑事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_lineEdit_Expansion_editingFinished();
        /**
         * @brief 第一层厚度编辑事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */        
        void on_lineEdit_FirThickness_editingFinished();
    private:
        /**
         * @brief 数据检查
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        bool checkValue();
        /**
         * @brief Set the Data To Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setDataToWidget();
        /**
         * @brief Get the Data From Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void getDataFromWidget();
    private:
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::LocalGroupInfoWidget* _ui = nullptr;
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief 网格尺寸对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKGeometryMeshSize* _obj = nullptr;
    };
}

#endif
