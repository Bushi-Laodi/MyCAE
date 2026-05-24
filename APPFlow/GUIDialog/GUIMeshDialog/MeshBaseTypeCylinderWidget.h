/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 *
 * @file MeshBaseTypeCylinderWidget.h
 * @brief 圆柱型局部区域尺寸
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 *
 */
#ifndef _MeshBaseTypeCylinderWidget_H
#define _MeshBaseTypeCylinderWidget_H

#include "MeshBaseTypeWidgetBase.h"

namespace Ui {
    class MeshBaseTypeCylinderWidget;
}

namespace Interface {
    class FITKRegionMeshSizeCylinder;
}

namespace GUI
{
    /**
     * @brief 圆柱型局部区域尺寸
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class MeshBaseTypeCylinderWidget :public MeshBaseTypeWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Mesh Base Type Cylinder Widget object
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshBaseTypeCylinderWidget(QWidget* parent);
        /**
         * @brief Destroy the Mesh Base Type Cylinder Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~MeshBaseTypeCylinderWidget();
        /**
         * @brief 初始化界面
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
         * @param[i]  obj            局部区域尺寸对象
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
         * @brief 清除局部区域尺寸边界背景颜色
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
         * @brief 保存数据槽函数
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
        Ui::MeshBaseTypeCylinderWidget* _ui = nullptr;
    };
}

#endif
