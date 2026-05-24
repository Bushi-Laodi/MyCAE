/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 *
 * @file MeshBaseWidget.h
 * @brief 局部区域尺寸设置主界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 *
 */
#ifndef _MeshBaseWidget_H
#define _MeshBaseWidget_H

#include "GUIMeshDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Ui {
    class MeshBaseWidget;
}

namespace Interface {
    class FITKRegionMeshSizeManager;
    class FITKAbstractRegionMeshSize;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class MeshBaseTypeWidgetBase;
    /**
     * @brief 局部区域尺寸设置主界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIMeshDialogAPI MeshBaseWidget :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Mesh Base Widget object
         * @param[i]  oper           操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshBaseWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Mesh Base Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~MeshBaseWidget();
        /**
         * @brief 初始化界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief 存储数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void saveValue();
    private slots:
        ;
        /**
         * @brief 局部区域尺寸类型选择槽函数
         * @param[i]  index          类型索引
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_comboBox_Type_activated(int index);
    private:
        /**
         * @brief 更新界面
         * @param[i]  newWidget      新界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void updateWidget(MeshBaseTypeWidgetBase* newWidget);
    private:
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
        Ui::MeshBaseWidget* _ui = nullptr;
        /**
         * @brief 当前局部区域尺寸对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKAbstractRegionMeshSize* _currentObj = nullptr;
        /**
         * @brief 局部区域尺寸管理器
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKRegionMeshSizeManager* _meshSizeManager = nullptr;
        /**
         * @brief 子界面对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshBaseTypeWidgetBase* _subWidget = nullptr;
    };
}

#endif
