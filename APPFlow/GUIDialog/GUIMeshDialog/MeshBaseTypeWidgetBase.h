/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 *
 * @file MeshBaseTypeWidgetBase.h
 * @brief 局部区域尺寸界面基类
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 *
 */
#ifndef _MeshBaseTypeWidgetBase_H
#define _MeshBaseTypeWidgetBase_H

#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Interface {
    class FITKAbstractRegionMeshSize;
}

namespace GUI
{
    class MeshBaseWidget;
    class MainWindow;

    /**
     * @brief 局部区域尺寸界面基类
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class MeshBaseTypeWidgetBase :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Mesh Base Type Widget Base object
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshBaseTypeWidgetBase(QWidget* parent);
        /**
         * @brief Destroy the Mesh Base Type Widget Base object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual ~MeshBaseTypeWidgetBase();
        /**
         * @brief 检查数据
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual bool checkValue() = 0;
        /**
         * @brief Set the Data To Widget object
         * @param[i]  obj            局部区域尺寸对象
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual bool setDataToWidget(Interface::FITKAbstractRegionMeshSize* obj) = 0;
        /**
         * @brief Get the Data From Widget object
         * @param[i]  obj            局部区域尺寸对象
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual bool getDataFromWidget(Interface::FITKAbstractRegionMeshSize* obj) = 0;
        /**
         * @brief 更新包围盒
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual void updateGeometryGraph() = 0;
    protected slots:
        ;
        /**
         * @brief 鼠标移动槽函数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotMouseMove();
    protected:
        /**
         * @brief 清除局部区域尺寸边界背景颜色
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual void clearBoundaryBackgroudColor() = 0;
    protected:
        /**
         * @brief 主窗口对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MainWindow* _mainWin = nullptr;
        /**
         * @brief 局部区域尺寸设置主界面对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshBaseWidget* _meshBaseWidget = nullptr;
        /**
         * @brief 临时局部区域尺寸对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKAbstractRegionMeshSize* _graphObj = nullptr;
    };
}

#endif
