/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 *
 * @file MeshBaseTypeBoxWidget.h
 * @brief 立方体型局部区域尺寸编辑界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 *
 */
#ifndef _MeshBaseTypeBoxWidget_H
#define _MeshBaseTypeBoxWidget_H

#include "MeshBaseTypeWidgetBase.h"

namespace Ui {
    class MeshBaseTypeBoxWidget;
}

namespace Interface {
    class FITKRegionMeshSizeBox;
}

namespace GUI
{
    /**
     * @brief 立方体型局部区域尺寸编辑界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class MeshBaseTypeBoxWidget :public MeshBaseTypeWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Mesh Base Type Box Widget object
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshBaseTypeBoxWidget(QWidget* parent);
        /**
         * @brief Destroy the Mesh Base Type Box Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~MeshBaseTypeBoxWidget();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
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
         * @param[i]  obj            局部区域尺寸对象
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        bool setDataToWidget(Interface::FITKAbstractRegionMeshSize* obj) override;
        /**
         * @brief Get the Data From Widget object
         * @param[i]  obj            局部区域尺寸
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        bool getDataFromWidget(Interface::FITKAbstractRegionMeshSize* obj) override;
        /**
         * @brief 更新包围盒
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void updateGeometryGraph() override;
    protected:
        /**
         * @brief 清除局部局域尺寸边界控件背景颜色
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void clearBoundaryBackgroudColor() override;
    private slots:
        ;
        /**
         * @brief 自适应按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_AutoSize_clicked();
        /**
         * @brief 存储数据槽函数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotSaveValue();
    private:
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::MeshBaseTypeBoxWidget* _ui = nullptr;
    };
}

#endif
